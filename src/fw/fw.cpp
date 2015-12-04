/*
 *  fw.cpp
 *
 *  Function Writer
 *
 *  Created by Brian Puchala.
 *	All rights reserved.
 *
 *
 */


//#include<iostream>
#include<fstream>
//#include<istream>
#include<cstdlib>
#include<cstring>
#include<vector>
//#include<getopt.h>
#include<boost/program_options.hpp>
#include<boost/filesystem.hpp>
//#include<cmath>
//#include<stdlib.h>
//#include<stdio.h>
//#include<iomanip>
//#include<time.h>
//#include<sstream>

#include "IntegrationToolsWriter/writer/PFunctionWriter.hh"
#include "IntegrationToolsWriter/version.hh"


template<typename T>
std::ostream& operator<<(std::ostream &sout, const std::vector<T> &v)
{
    for( int i=0; i<v.size(); i++)
        sout << v[i] << " ";
    return sout;
}

void print_examples( std::ostream &sout)
{
    sout << "\n\nExamples: \n\n";
    
    sout << "A polynomial function:\n";
    sout << "fw -n \"MyFunc\" -v \"x\" \"y\" -d \"x variable\" \"y variable\"  --sym \"x^3 + x^2*y + x*y^2 + y^3\" --grad --hess -l \"/path/to/MyLibrary\"\n\n";
    
    sout << "A piece-wise function:\n";
    sout << "fw -n \"MyPieceWiseFunc\" -v \"x\" \"y\" -d \"x variable\" \"y variable\"  --piecewise \'[ { \"func\": \"pow(x,2) + pow(y,2)\", \"cond\":[\"pow(x,2) + pow(y,2)<0.5*0.5\"] }, { \"func\": \"sqrt(pow(x,2) + pow(y,2)) - 0.25\", \"cond\":[\"pow(x,2) + pow(y,2)>=0.5*0.5\"] } ]\' --grad --hess -l \"/path/to/MyLibrary\"\n\n";
}

const size_t ERROR_IN_COMMAND_LINE = 1; 
const size_t SUCCESS = 0; 
const size_t ERROR_UNHANDLED_EXCEPTION = 2; 

int main(int argc, char *argv[])
{
    // input variables
    std::string name, location, intype, outtype, sym, piecewise;
    std::vector< std::string> var_name, var_desc;
    
    namespace po = boost::program_options;
    namespace fs = boost::filesystem;
     
    po::variables_map vm; 
    
    try 
    {
        /// Set command line options using boost program_options
        po::options_description desc("Options"); 
        desc.add_options() 
          ("help", "Write help documentation, including examples")
          ("version", "Write version info")
          ("name,n", po::value<std::string>(&name)->required(), "Function name") 
          ("location,l", po::value<std::string>(&location)->default_value("."), "Location to write function") 
          ("sym", po::value<std::string>(&sym)  , "Symbolic expression") 
          ("piecewise", po::value<std::string>(&piecewise)  , "Expressions defining a piecewise function, listed in JSON") 
          ("var,v", po::value<std::vector<std::string> >(&var_name)->multitoken()->required(), "Variable names")
          ("description,d", po::value<std::vector<std::string> >(&var_desc)->multitoken()->required(), "Variable descriptions")
          ("grad", "Include gradient calculation")
          ("hess", "Include Hessian calculation");
        
        //("intype,i", po::value<std::string>(&intype), "Function input type") 
        //("outtype,o", po::value<std::string>(&outtype), "Function output type") 
          
        
        
        try 
        { 
            po::store(po::parse_command_line(argc, argv, desc), vm); // can throw 

            /** --help option 
            */ 
            if ( vm.count("help")  ) 
            { 
                std::cout << "fw: Function Writer Usage" << std::endl << desc << std::endl;
                print_examples(std::cout);
                return SUCCESS; 
            } 
            
            /** --version option 
            */ 
            if ( vm.count("version")  ) 
            { 
                std::cout << "fw: Function Writer" << std::endl; 
                std::cout << "  version: " << PRISMS::IntegrationToolsWriter_version_id() << std::endl; 
                std::cout << "  url: " << PRISMS::IntegrationToolsWriter_repo_url() << std::endl; 
                std::cout << "  commit: " << PRISMS::IntegrationToolsWriter_commit_id() << std::endl; 
                return SUCCESS; 
            }
            
            if( !(vm.count("sym") != vm.count("piecewise")) )
            {
                throw po::error("Error writing function. Either choose (--sym) or (--piecewise).");
            } 

            po::notify(vm); // throws on error, so do after help in case 
                          // there are any problems 
        } 
        catch(po::error& e) 
        { 
          std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
          std::cerr << desc << std::endl; 
          //print_examples(std::cerr);
          return ERROR_IN_COMMAND_LINE; 
        }
    }
    catch(std::exception& e) 
    { 
        std::cerr << "Unhandled Exception reached the top of main: " 
                  << e.what() << ", application will now exit" << std::endl; 
        return ERROR_UNHANDLED_EXCEPTION; 

    } 
    
    
    PRISMS::PFunctionWriter writer(name);
    std::ofstream outfile;
    
    fs::path fileloc(location);
    fileloc /= (name + ".hh");
    
    std::cout << "path: " << fileloc.c_str() << std::endl;
    
    outfile.open( fileloc.c_str() );
    
    writer.set_var( var_name, var_desc);
    
    //if( vm.count("intype") ) writer.set_intype( intype );
    //if( vm.count("outtype") )writer.set_outtype( outtype );
    vm.count("grad") ? writer.grad_on() : writer.grad_off();
    vm.count("hess") ? writer.hess_on() : writer.hess_off();
    
    if( vm.count("sym") )
    {
        writer.head(outfile);
        try
        {
            writer.sym2code( sym , outfile);
        }
        catch(std::invalid_argument& err)
        {
            std::cout << "Error parsing symbolic expression." << std::endl;
            std::cout << "  Unknown symbolic variable(s) in input expression." << std::endl;
            std::cout << "  " << err.what() << std::endl;
            outfile.close();
            fs::remove(fileloc);
            return 1;
        }
        writer.foot(outfile);
    }
    else if (vm.count("piecewise"))
    {
        writer.head(outfile, true);
        try
        {
            writer.piecewise2code( piecewise, outfile);
        }
        catch(std::invalid_argument& err)
        {
            std::cout << "Error parsing piecewise function expression." << std::endl;
            std::cout << "  Unknown symbolic variable(s) in input expression." << std::endl;
            std::cout << "  " << err.what() << std::endl;
            outfile.close();
            fs::remove(fileloc);
            return 1;
        }
        writer.foot(outfile);
    }
    else
    {
        throw po::error("Error writing function. Either choose (--sym) or (--piecewise).");
    }
    
    return 0;
}
