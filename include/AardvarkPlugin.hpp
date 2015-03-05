/*
 * AardvarkPlugin.hpp
 *
 *  Created on: 25.02.2015
 *      Author: Dave
 */

#ifndef AARDVARKPLUGIN_HPP_
#define AARDVARKPLUGIN_HPP_

#define UDS_REGISTER_TO_RSD_PATH "/tmp/RsdRegister.uds"
#define UDS_COM_PATH "/tmp/AardvarkPlugin.uds"

#include "UdsServer.hpp"
#include "UdsRegClient.hpp"


class AardvarkPlugin {

	public:
		AardvarkPlugin();
		~AardvarkPlugin();


		void startCommunication();
		void registerToRSD();

		static list<string*>* getFuncList(){return funcList;}


	private:
		UdsServer* comServer;
		UdsRegClient* regClient;

		bool regClientReady;
		bool comServerReady;

		static list<string*>* funcList;



};

#endif /* AARDVARKPLUGIN_HPP_ */
