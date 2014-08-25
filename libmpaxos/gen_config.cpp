/*
 *Author:loli
 *Date: 2014/8/21
 *Usage:Genrate config-*.yaml
 *
 */
#include <fstream>
#include <stdlib.h>
#include <yaml.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include "tools.hpp"

int main(int argc, char *argv[])
{
	if(argc < 2){
		printf("Usage: %s Numbers of Nodes\n",argv[0]);
		return 0;
	}
	YAML::Node config;
	int number = atoi(argv[1]);

	for(int i = 0 ;i < number; i++){
		YAML::Node nodes;
		nodes["name"] = "node" + int2string(i + 1);
		nodes["addr"] = "localhost";
	//	sprintf(buf, "%d", i+8881);
	//	buf_str =  buf;
		nodes["port"] = int2string(8881 + i);

		config["host"].push_back(nodes);
	}

//	const std::string username = config["username"].as<std::string>();
	printf("OK!\n");
	std::string number_str = argv[1];
	std::string filename = "config/localhost-" + number_str + ".yaml";
	std::ofstream fout(filename.c_str());

	fout << config;
}
