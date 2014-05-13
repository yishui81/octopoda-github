/*
 * Request.cpp
 *
 *  Created on: 2014年5月9日
 *      Author: jacky
 */

#include "Request.h"
#include "Connector.h"

Request::Request(Connector* iostream) {
	// TODO Auto-generated constructor stub
	this->m_connector = iostream;
}

Request::~Request() {
	// TODO Auto-generated destructor stub
}

