/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#include <QApplication>
#include <iostream>

#include "mainwindow.h"

int main(int argc, char* argv[])
{
	if (argc != 3 && argc != 2)
	{
		std::cout << "Use 'ultrapythonui --help'"<<std::endl;
		exit(-1);
	}
	if (std::string(argv[1]) == "--help")
	{
		std::cout << "Usage 'ultrapythonui --remote /name'  NOTE: the name is without rpc and port name usually is /grabber"<<std::endl;
		exit(-1);
	}

	if (argc != 3)
	{
		std::cout << "Missing remote port name, 'use ultrapythonui --remote /name'"<<std::endl;
		exit(-1);
	}
	if (std::string(argv[1]) != "--remote")
	{
		std::cout << "Missing remote port name, 'use ultrapythonui --remote /name' NOTE that the name is without rpc"<<std::endl;
		exit(-1);
	}
	if (std::string(argv[2]).front() != '/')
	{
		std::cout << "Wrong remote port name syntax, 'use ultrapythonui --remote /name' NOTE that the name is without rpc"<<std::endl;
		exit(-1);
	}

	QApplication a(argc, argv);
	MainWindow w(argv[2]);
	w.show();
	return a.exec();
}
