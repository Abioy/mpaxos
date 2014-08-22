/*
 * mpaxos-config.cpp
 *
 * Created on: Aug 22, 2014
 * Author: loli
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <time.h>
#include <unistd.h>
#include <json/json.h>
#include <apr_time.h>
#include <yaml.h>
#include <string>
#include <iostream>

#include "mpaxos/mpaxos.h"
#include "comm.h"
#include "view.h"
#include "utils/hostname.h"
#include "utils/logger.h"



int flush__;

int mpaxos_config_load(const char *cf) {
    LOG_INFO("loading config file %s ...\n", cf);
	
	YAML::Node config;

    if (cf == NULL) {
		config = YAML::LoadFile("config/localhost-1.yaml");
    } else {
		config = YAML::LoadFile(cf);
    }
    if (config == NULL) {
        printf("cannot open config file: %s.\n", cf);
        return -1;
    }

	YAML::Node nodes = config["host"];

    for (std::size_t i = 0; i < nodes.size(); i++) {

		YAML::Node node = nodes[i];

		const char* name = node["name"].as<std::string>().c_str();
		const char* addr = node["addr"].as<std::string>().c_str();
        int port = node["port"].as<int>();

        // set a node in view
        set_node(name, addr, port);
    }
    
	YAML::Node config_nodename = config["nodename"];
    if (config_nodename != NULL) {
        const char* nodename = config_nodename["nodename"].as<std::string>().c_str();
        mpaxos_config_set("nodename", nodename); 
	}

    LOG_INFO("config file loaded\n");
    return 0;
}

int mpaxos_config_set(const char *key, const char *value) {
    if (strcmp(key, "nodename") == 0) {
        set_nodename(value);                 
    }
    return 0;
}

int mpaxos_config_get(const char *key, char** value) {
    return 0;
}
