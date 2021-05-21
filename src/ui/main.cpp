/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
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
