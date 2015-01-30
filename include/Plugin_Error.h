/*
 * Plugin_Error.h
 *
 *  Created on: 28.01.2015
 *      Author: dnoack
 */

#ifndef INCLUDE_PLUGIN_ERROR_H_
#define INCLUDE_PLUGIN_ERROR_H_

#include <exception>
#include <string>
#include <sstream>

using namespace std;

class PluginError : exception{


	public:
		PluginError(const char* msg, char* file, int line)
		{
			this->msg = msg;
			this->file = file;
			this->line = line;
			oStream = new ostringstream();
		}


		~PluginError() throw()
		{
			delete(oStream);
		}


		char* get() const throw()
		{
			*oStream << "An error was thrown in file: " << file << " at line: " << line << " ### " << msg ;
			return strdup((char*)(oStream->str().c_str()));
		}


	private:
		const char* msg;
		char* file;
		int line;
		ostringstream* oStream;



};

//string PluginError::msgOut;

//#define throw_PluginError(msg) throw PluginError(msg , __FILE__, __LINE__);



#endif /* INCLUDE_PLUGIN_ERROR_H_ */