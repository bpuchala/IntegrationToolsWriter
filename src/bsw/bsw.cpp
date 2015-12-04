/*
 *  bsw.cpp
 *
 *  Basis Set Writer
 *
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

#include "IntegrationToolsWriter/writer/PBasisSetWriter.hh"
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
    
    sout << "A monomial basis set:\n";
    sout << "bsw -n \"Monomial\" -d \"Monomials up to 30\" -v \"x\" --index \"i\" -e \"x^i\" -m 30 --grad --hess -l \"/path/to/MyLibrary\"\n\n";
    
    sout << "A Chebyshev polynomial basis set:\n";
    sout << "bsw -n \"Chebyshev\" -d \"Chebyshev Polynomials up to term 30\"  -v \"x\" -e \"2*x*phi1 - phi0\" --init \"1.0\" \"x\" --sym \"phi0\" \"phi1\" -m 30 --grad --hess -l \"/path/to/MyLibrary\"\n\n";
}

const size_t ERROR_IN_COMMAND_LINE = 1; 
const size_t SUCCESS = 0; 
const size_t ERROR_UNHANDLED_EXCEPTION = 2; 

int main(int argc, char *argv[])
{
    // input variables
    int max;
    std::string name, description, location, intype, outtype, var, e_gen, index;
    std::vector< std::string> sym, init;
    
    namespace po = boost::program_options;
    namespace fs = boost::filesystem;
     
    po::variables_map vm; 
    
    try 
    {
        /// Set command line options using boost program_options
        po::options_description desc("Options"); 
        desc.add_options() 
          ("help", "Write help documentation")
          ("version", "Write version info")
          ("name,n", po::value<std::string>(&name)->required(), "Basis set name") 
          ("description,d", po::value<std::string>(&description)->required(), "Basis set description") 
          ("location,l", po::value<std::string>(&location)->default_value("."), "Location to write basis set") 
          ("var,v", po::value<std::string>(&var)->required(), "Variable symbol")
          ("index", po::value<std::string>(&index), "Index symbol")
          ("expression,e", po::value<std::string>(&e_gen)->required(), "Generating expression")
          ("sym", po::value<std::vector<std::string> >(&sym)->multitoken(), "Symbols for previous basis functions") 
          ("init", po::value<std::vector<std::string> >(&init)->multitoken(), "Initial basis functions")
          ("max,m", po::value<int>(&max)->required(), "Max size of basis set")
          ("grad", "Include gradient calculation")
          ("hess", "Include Hessian calculation");
        
        //("intype,i", po::value<std::string>(&intype), "Basis set input type") 
        //("outtype,o", po::value<std::string>(&outtype), "Basis set output type")
          
        
        try 
        { 
            po::store(po::parse_command_line(argc, argv, desc), vm); // can throw 

            /** --help option 
            */ 
            if ( vm.count("help")  ) 
            { 
                std::cout << "bsw: Basis Set Writer Usage" << std::endl << desc << std::endl; 
                print_examples(std::cout);
                return SUCCESS; 
            }
            
            /** --version option 
            */ 
            if ( vm.count("version")  ) 
            { 
                std::cout << "bsw: Basis Set Writer" << std::endl; 
                std::cout << "  version: " << PRISMS::IntegrationToolsWriter_version_id() << std::endl; 
                std::cout << "  url: " << PRISMS::IntegrationToolsWriter_repo_url() << std::endl; 
                std::cout << "  commit: " << PRISMS::IntegrationToolsWriter_commit_id() << std::endl; 
                return SUCCESS; 
            }
            
            if( vm.count("index") )
            {
                if( vm.count("init") || vm.count("sym") )
                {
                    throw po::error("Error writing basis set. Either set (--index) or (--init and --sym).");
                }
            }
            else
            {
                if( !(vm.count("init") && vm.count("sym")) )
                {
                    //desc = "Error writing basis set. Either set (--index) or (--init and --sym).";
                    throw po::error("Error writing basis set. Either set (--index) or (--init and --sym).");
                }
            }

            po::notify(vm); // throws on error, so do after help in case 
                          // there are any problems 
        } 
        catch(po::error& e) 
        { 
          std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
          std::cerr << desc << std::endl; 
          return ERROR_IN_COMMAND_LINE; 
        }
    }
    catch(std::exception& e) 
    { 
        std::cerr << "Unhandled Exception reached the top of main: " 
                  << e.what() << ", application will now exit" << std::endl; 
        return ERROR_UNHANDLED_EXCEPTION; 

    } 
    
    
    PRISMS::PBasisSetWriter writer(name, description);
    std::ofstream outfile;
    
    fs::path fileloc(location);
    fileloc /= (name + ".hh");
    
    std::cout << "path: " << fileloc.c_str() << std::endl;
    
    outfile.open( fileloc.c_str() );
    
    
    //if( vm.count("intype") ) writer.set_intype( intype );
    //if( vm.count("outtype") )writer.set_outtype( outtype );
    vm.count("grad") ? writer.grad_on() : writer.grad_off();
    vm.count("hess") ? writer.hess_on() : writer.hess_off();
    
    if( vm.count("index"))
    {
        try
        {
            writer.sym2code(e_gen, var, index, max, outfile);
        }
        catch(std::invalid_argument& err)
        {
            std::cout << "Error parsing generating expression." << std::endl;
            std::cout << "  Unknown symbolic variable(s) in input expression." << std::endl;
            std::cout << "  " << err.what() << std::endl;
            outfile.close();
            fs::remove(fileloc);
            return 1;
        }
    }
    else
    {
        std::cout << "e_gen: " << e_gen << std::endl;
        std::cout << "var: " << var << std::endl;
        std::cout << "init: " << init << std::endl;
        std::cout << "sym: " << sym << std::endl;
        std::cout << "max: " << max << std::endl;
        
        
        try
        {
            writer.sym2code(e_gen, var, init, sym, max, outfile);
        }
        catch(std::invalid_argument& err)
        {
            std::cout << "Error parsing generating expression." << std::endl;
            std::cout << "  Unknown symbolic variable(s) in input expression." << std::endl;
            std::cout << "  " << err.what() << std::endl;
            outfile.close();
            fs::remove(fileloc);
            return 1;
        }
    }
    
    return 0;
}
