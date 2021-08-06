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

#include "mainwindow.h"

#include <QMessageBox>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <ratio>
#include <thread>

#include "./ui_mainwindow.h"
#include "common.h"

MainWindow::MainWindow(const std::string& remotePort, QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	initYarp(remotePort);
	if (!readAndShowValues())
	{
		QMessageBox msgBox;
		msgBox.setText("Can not read control values.");
		msgBox.exec();
		// exit(-1);
	}

	fpsShow_ = std::make_shared<std::thread>(&MainWindow::showFps, this);
}

MainWindow::~MainWindow()
{
	activeShowFps_ = false;
	fpsShow_->join();
	delete ui;
}

bool MainWindow::readAndShowValues()
{
	if (!grabber_)
	{
		return false;
	}

	int feature = (int)YARP_FEATURE_GAIN;
	double gainValue;
	bool res = grabber_->getFeature(feature, &gainValue);
	if (!res)
	{
		return false;
	}
	std::stringstream ss;
	ss << std::setprecision(2) << gainValue * 100 << "%";
	ui->gainNorm->setText(ss.str().c_str());
	ui->gainSlider->setValue(gainValue * 100);

	feature = (int)YARP_FEATURE_GAIN_ABSOLUTE;
	double gainAbsoluteValue;
	res = grabber_->getFeature(feature, &gainAbsoluteValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << (int)gainAbsoluteValue;
	ui->gainAbs->setText(ss.str().c_str());

	feature = (int)YARP_FEATURE_BRIGHTNESS;
	double brightnessValue;
	res = grabber_->getFeature(feature, &brightnessValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << std::setprecision(2) << brightnessValue * 100 << "%";
	ui->brightnessNorm->setText(ss.str().c_str());
	ui->brightnessSlider->setValue(brightnessValue * 100);

	feature = (int)YARP_FEATURE_BRIGHTNESS_ABSOLUTE;
	double brightnessAbsoluteValue;
	res = grabber_->getFeature(feature, &brightnessAbsoluteValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << (int)brightnessAbsoluteValue;
	ui->brightnessAbs->setText(ss.str().c_str());

	feature = (int)YARP_FEATURE_CONTRAST;
	double contrastValue;
	res = grabber_->getFeature(feature, &contrastValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << std::setprecision(2) << contrastValue * 100 << "%";
	ui->contrastNorm->setText(ss.str().c_str());
	ui->contrastSlider->setValue(contrastValue * 100);

	feature = (int)YARP_FEATURE_CONTRAST_ABSOLUTE;
	double contrastAbsoluteValue;
	res = grabber_->getFeature(feature, &contrastAbsoluteValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << (int)contrastAbsoluteValue;
	ui->contrastAbs->setText(ss.str().c_str());

	feature = (int)YARP_FEATURE_EXPOSURE;
	double exposureValue;
	res = grabber_->getFeature(feature, &exposureValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << std::setprecision(2) << exposureValue * 100 << "%";
	ui->exposureNorm->setText(ss.str().c_str());
	ui->exposureSlider->setValue(exposureValue * 100);

	feature = (int)YARP_FEATURE_EXPOSURE_ABSOLUTE;
	double exposureAbsoluteValue;
	res = grabber_->getFeature(feature, &exposureAbsoluteValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << (int)exposureAbsoluteValue;
	ui->exposureAbs->setText(ss.str().c_str());

	feature = (int)YARP_FEATURE_RED_GAIN;
	double redGainValue;
	res = grabber_->getFeature(feature, &redGainValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << std::setprecision(2) << redGainValue * 100 << "%";
	ui->redGainNorm->setText(ss.str().c_str());
	ui->redGainSlider->setValue(redGainValue * 100);

	feature = (int)YARP_FEATURE_RED_GAIN_ABSOLUTE;
	double redGainAbsoluteValue;
	res = grabber_->getFeature(feature, &redGainAbsoluteValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << (int)redGainAbsoluteValue;
	ui->redGainAbs->setText(ss.str().c_str());

	feature = (int)YARP_FEATURE_BLUE_GAIN;
	double blueGainValue;
	res = grabber_->getFeature(feature, &blueGainValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << std::setprecision(2) << blueGainValue * 100 << "%";
	ui->blueGainNorm->setText(ss.str().c_str());
	ui->blueGainSlider->setValue(blueGainValue * 100);

	feature = (int)YARP_FEATURE_BLUE_GAIN_ABSOLUTE;
	double blueGainAbsoluteValue;
	res = grabber_->getFeature(feature, &blueGainAbsoluteValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << (int)blueGainAbsoluteValue;
	ui->blueGainAbs->setText(ss.str().c_str());

	feature = (int)YARP_FEATURE_GREEN_GAIN;
	double greenGainValue;
	res = grabber_->getFeature(feature, &greenGainValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << std::setprecision(2) << greenGainValue * 100 << "%";
	ui->greenGainNorm->setText(ss.str().c_str());
	ui->greenGainSlider->setValue(greenGainValue * 100);

	feature = (int)YARP_FEATURE_GREEN_GAIN_ABSOLUTE;
	double greenGainAbsoluteValue;
	res = grabber_->getFeature(feature, &greenGainAbsoluteValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << (int)greenGainAbsoluteValue;
	ui->greenGainAbs->setText(ss.str().c_str());

	feature = (int)YARP_FEATURE_HONOR_FPS;
	double honorFpsValue;
	res = grabber_->getFeature(feature, &honorFpsValue);
	if (!res)
	{
		return false;
	}
	if (honorFpsValue == 0)
		ui->honorFPS->setChecked(false);
	else
		ui->honorFPS->setChecked(true);

	feature = (int)YARP_FEATURE_FPS;
	double fpsValue{44};
	res = grabber_->getFeature(feature, &fpsValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << fpsValue;
	ui->fps->setText(ss.str().c_str());

	feature = (int)YARP_FEATURE_SUBSAMPLING;
	double resolution{0};
	res = grabber_->getFeature(feature, &resolution);
	if (!res)
	{
		return false;
	}
	ss.str("");
	if (resolution == 0)
		ss << "High";
	else
		ss << "Low";
	ui->resolution->setText(ss.str().c_str());

	return true;
}

bool MainWindow::initYarp(const std::string& remotePort)
{
	if (!yarp::os::NetworkBase::checkNetwork(2))
	{
		QMessageBox msgBox;
		msgBox.setText("Yarp yarpserver not found.\nPlease activate yarpserver and retry.");
		msgBox.exec();
		exit(-1);
	}

	yarp::os::Property property;
	property.put("device", "remote_grabber");
	property.put("local", "/xxx");
	property.put("remote", remotePort + "/rpc");

	if (!device_.open(property))
	{
		QMessageBox msgBox;
		msgBox.setText("Unable to open device.");
		msgBox.exec();
		exit(-1);
	}
	device_.view(grabber_);

	if (!grabber_)
	{
		QMessageBox msgBox;
		msgBox.setText("Unable to view device.");
		msgBox.exec();
		exit(-1);
		return false;
	}

	return true;
}

void MainWindow::on_gainSlider_sliderReleased()
{
	int feature = (int)YARP_FEATURE_GAIN;
	auto value = ui->gainSlider->value();
	double gainValue = (double)value / 100;
	std::cout << "Slider:" << gainValue << std::endl;

	bool res = grabber_->setFeature(feature, gainValue);
	if (!res)
	{
		emitError("green gain");
		return;
	}
	readAndShowValues();
}

void MainWindow::on_exposureSlider_sliderReleased()
{
	int feature = (int)YARP_FEATURE_EXPOSURE;
	auto value = ui->exposureSlider->value();
	double exposureValue = (double)value / 100;
	std::cout << "Slider:" << exposureValue << std::endl;

	bool res = grabber_->setFeature(feature, exposureValue);
	if (!res)
	{
		emitError("exposure");
		return;
	}
	readAndShowValues();
}

void MainWindow::on_brightnessSlider_sliderReleased()
{
	int feature = (int)YARP_FEATURE_BRIGHTNESS;
	auto value = ui->brightnessSlider->value();
	double brigthnessValue = (double)value / 100;
	std::cout << "Slider:" << brigthnessValue << std::endl;

	bool res = grabber_->setFeature(feature, brigthnessValue);
	if (!res)
	{
		emitError("brightness");
		return;
	}
	readAndShowValues();
}

void MainWindow::on_contrastSlider_sliderReleased()
{
	int feature = (int)YARP_FEATURE_CONTRAST;
	auto value = ui->contrastSlider->value();
	double contrastValue = (double)value / 100;
	std::cout << "Slider:" << contrastValue << std::endl;

	bool res = grabber_->setFeature(feature, contrastValue);
	if (!res)
	{
		emitError("contrast");
		return;
	}
	readAndShowValues();
}

void MainWindow::on_redGainSlider_sliderReleased()
{
	int feature = (int)YARP_FEATURE_RED_GAIN;
	auto value = ui->redGainSlider->value();
	double redGainValue = (double)value / 100;
	std::cout << "Slider:" << redGainValue << std::endl;

	bool res = grabber_->setFeature(feature, redGainValue);
	if (!res)
	{
		emitError("red gain");
		return;
	}
	readAndShowValues();
}

void MainWindow::on_blueGainSlider_sliderReleased()
{
	int feature = (int)YARP_FEATURE_BLUE_GAIN;
	auto value = ui->blueGainSlider->value();
	double blueGainValue = (double)value / 100;
	std::cout << "Slider:" << blueGainValue << std::endl;

	bool res = grabber_->setFeature(feature, blueGainValue);
	if (!res)
	{
		emitError("blue gain");
		return;
	}
	readAndShowValues();
}

void MainWindow::on_greenGainSlider_sliderReleased()
{
	int feature = (int)YARP_FEATURE_GREEN_GAIN;
	auto value = ui->greenGainSlider->value();
	double greenGainValue = (double)value / 100;
	std::cout << "Slider:" << greenGainValue << std::endl;

	bool res = grabber_->setFeature(feature, greenGainValue);
	if (!res)
	{
		emitError("green gain");
		return;
	}
	readAndShowValues();
}

void MainWindow::emitError(const std::string& text)
{
	std::stringstream ss;
	ss << "Not able to set ";
	ss << text;
	ss << std::endl;
	ss << "Check remote yarpdev device.";

	QMessageBox msgBox;
	msgBox.setText(ss.str().c_str());
	msgBox.exec();
}

void MainWindow::on_honorFPS_stateChanged(int arg1)
{
	int feature = (int)YARP_FEATURE_HONOR_FPS;
	bool value = ui->honorFPS->isChecked();

	bool res = grabber_->setFeature(feature, value);

	readAndShowValues();
}

using namespace std::chrono_literals;
void MainWindow::showFps()
{
	while (activeShowFps_)
	{
		std::this_thread::sleep_for(1s);
		int feature = (int)YARP_FEATURE_FPS;
		double fpsValue{44};
		bool res = grabber_->getFeature(feature, &fpsValue);
		if (!res)
		{
			return;
		}
		std::stringstream ss;
		ss.str("");
		ss << fpsValue;
		ui->fps->setText(ss.str().c_str());
	}
}
void MainWindow::on_actionAbout_triggered()
{
	std::stringstream ss;
	ss<<"This UI can only be used to control the UltraPython board."<<std::endl; 
	ss<<std::endl; 
	ss<<"luca.tricerri@iit.it, 2021"<<std::endl; 
    QMessageBox msgBox;
    msgBox.setText(ss.str().c_str());
    msgBox.exec();
}

