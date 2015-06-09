#include "RegClient.hpp"


struct sockaddr_un RegClient::address;
socklen_t RegClient::addrlen;


RegClient::RegClient(const char* pluginName, int pluginNumber, const char* regPath, const char* comPath)
{
	this->regPath = regPath;
	globalDom = NULL;
	comPoint = NULL;
	optionflag = 1;
	currentMsgId = NULL;
	state = NOT_ACTIVE;
	address.sun_family = AF_UNIX;
	globalDom = new Document();

	strncpy(address.sun_path, regPath, strlen(regPath));
	addrlen = sizeof(address);

	connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);

	if(connection_socket < 0)
		throw Error(-1100, "Could not create connection_socket");


	plugin = new Plugin(pluginName, pluginNumber ,comPath);
	json = new JsonRPC();
}


RegClient::~RegClient()
{
	if(comPoint != NULL)
		delete comPoint;

	delete plugin;
	delete json;
	delete globalDom;
}


void RegClient::connectToRSD()
{
	if( connect(connection_socket, (struct sockaddr*)&address, addrlen) != 0 )
		throw Error(-1101, "Could not connect to RSD.\n");
	else
		comPoint = new ComPoint(connection_socket, this, plugin->getPluginNumber());
}


void RegClient::unregisterFromRSD()
{
	//TODO: send a json rpc which tells the RSD that the plugin is going to shutdown
}


void RegClient::process(RPCMsg* msg)
{
	const char* response = NULL;

	try
	{
		json->parse(globalDom, msg->getContent());
		currentMsgId = json->tryTogetId(globalDom);

		if(json->isError(globalDom))
			throw Error(-1102, "Received json rpc error response.");

		switch(state)
		{
			case NOT_ACTIVE:
				//check for announce ack then switch state to announced
				//and send register msg
				if(handleAnnounceACKMsg())
				{
					state = ANNOUNCED;
					response = createRegisterMsg();
					comPoint->transmit(response, strlen(response));
				}
				break;
			case ANNOUNCED:
				if(handleRegisterACKMsg())
				{
					state = REGISTERED;
					response = createPluginActiveMsg();
					comPoint->transmit(response, strlen(response));
				}
				//check for register ack then switch state to active
				break;
			case ACTIVE:
				//maybe heartbeat check
				break;
			case BROKEN:
				//clean up an set state to NOT_ACTIVE
				state = NOT_ACTIVE;
				break;
			default:
				//something went completely wrong
				state = BROKEN;
				break;
		}
		delete msg;
	}
	catch(Error &e)
	{
		delete msg;
		state = BROKEN;
		close(connection_socket);
	}
}


void RegClient::sendAnnounceMsg()
{
	Value method;
	Value params;
	Value id;
	const char* announceMsg = NULL;
	Document* requestDOM = json->getRequestDOM();

	try
	{
		method.SetString("announce");
		params.SetObject();
		params.AddMember("pluginName", StringRef(plugin->getName()->c_str()), requestDOM->GetAllocator());
		params.AddMember("pluginNumber", plugin->getPluginNumber(), requestDOM->GetAllocator());
		params.AddMember("udsFilePath", StringRef(plugin->getUdsFilePath()->c_str()), requestDOM->GetAllocator());
		id.SetInt(1);

		announceMsg = json->generateRequest(method, params, id);
		comPoint->transmit(announceMsg, strlen(announceMsg));
	}
	catch(Error &e)
	{
		throw;
	}
}



bool RegClient::handleAnnounceACKMsg()
{
	Value* resultValue = NULL;
	const char* resultString = NULL;
	bool result = false;

	try
	{
		resultValue = json->tryTogetResult(globalDom);
		if(resultValue->IsString())
		{
			resultString = resultValue->GetString();
			if(strcmp(resultString, "announceACK") == 0)
				result = true;
		}
		else
		{
			throw Error(-1103, "Awaited announceACK.");
		}
	}
	catch(Error &e)
	{
		throw;
	}
	return result;
}


const char* RegClient::createRegisterMsg()
{
	Value method;
	Value params;
	Value functionArray;

	list<string*>* funcList;
	string* functionName;
	Document* requestDOM = json->getRequestDOM();

	//get methods from plugin
	funcList = AardvarkPlugin::getFuncList();
	method.SetString("register");
	params.SetObject();
	functionArray.SetArray();

	for(list<string*>::iterator ifName = funcList->begin(); ifName != funcList->end(); )
	{
		functionName = *ifName;
		functionArray.PushBack(StringRef(functionName->c_str()), requestDOM->GetAllocator());
		ifName = funcList->erase(ifName);
	}

	params.AddMember("functions", functionArray, requestDOM->GetAllocator());

	return json->generateRequest(method, params, *currentMsgId);
}


bool RegClient::handleRegisterACKMsg()
{
	const char* resultString = NULL;
	Value* resultValue = NULL;
	bool result = false;

	try
	{
		resultValue = json->tryTogetResult(globalDom);
		if(resultValue->IsString())
		{
			resultString = resultValue->GetString();
			if(strcmp(resultString, "registerACK") == 0)
				result = true;
		}
		else
		{
			throw Error(-1104, "Awaited registerACK.");
		}
	}
	catch(Error &e)
	{
		throw;
	}
	return result;
}


const char* RegClient::createPluginActiveMsg()
{
	Value method;
	Value* params = NULL;
	Value* id = NULL;
	const char* msg = NULL;

	method.SetString("pluginActive");
	msg = json->generateRequest(method, *params, *id);

	return msg;
}

