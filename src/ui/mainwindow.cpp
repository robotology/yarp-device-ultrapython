#include "mainwindow.h"

#include <QMessageBox>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <ratio>
#include <thread>

#include "./ui_mainwindow.h"

MainWindow::MainWindow(const std::string& remotePort, QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	initYarp(remotePort);
	if (!readAndShowValues())
	{
		QMessageBox msgBox;
		msgBox.setText("Can not read control values.");
		msgBox.exec();
		//exit(-1);
	}
}

MainWindow::~MainWindow()
{
	delete ui;
}

bool MainWindow::readAndShowValues()
{
	if (!grabber_)
	{
		return false;
	}

	cameraFeature_id_t feature = (cameraFeature_id_t)YARP_FEATURE_GAIN;
	double gainValue;
	bool res = grabber_->getFeature(feature, &gainValue);
	if (!res)
	{
		return false;
	}
	std::stringstream ss;
	ss << std::setprecision(2) << gainValue;
	ui->gainAbs->setText(ss.str().c_str());
	ui->gainSlider->setValue(gainValue * 100);

	feature = (cameraFeature_id_t)YARP_FEATURE_BRIGHTNESS;
	double brightnessValue;
	res = grabber_->getFeature(feature, &brightnessValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << std::setprecision(2) << brightnessValue;
	ui->brightnessAbs->setText(ss.str().c_str());
	ui->brightnessSlider->setValue(brightnessValue * 100);

	feature = (cameraFeature_id_t)YARP_FEATURE_EXPOSURE;
	double exposureValue;
	res = grabber_->getFeature(feature, &exposureValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << std::setprecision(2) << exposureValue;
	ui->exposureAbs->setText(ss.str().c_str());
	ui->exposureSlider->setValue(exposureValue * 100);

	feature = (cameraFeature_id_t)YARP_FEATURE_CAPTURE_SIZE;
	double redGainValue;
	res = grabber_->getFeature(feature, &redGainValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << std::setprecision(2) << redGainValue;
	ui->redGainAbs->setText(ss.str().c_str());
	ui->redGainSlider->setValue(redGainValue * 100);

	feature = (cameraFeature_id_t)YARP_FEATURE_MIRROR;
	double blueGainValue;
	res = grabber_->getFeature(feature, &blueGainValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << std::setprecision(2) << blueGainValue;
	ui->blueGainAbs->setText(ss.str().c_str());
	ui->blueGainSlider->setValue(blueGainValue * 100);

	feature = (cameraFeature_id_t)YARP_FEATURE_CAPTURE_QUALITY;
	double greenGainValue;
	res = grabber_->getFeature(feature, &greenGainValue);
	if (!res)
	{
		return false;
	}
	ss.str("");
	ss << std::setprecision(2) << greenGainValue;
	ui->greenGainAbs->setText(ss.str().c_str());
	ui->greenGainSlider->setValue(greenGainValue * 100);

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
	cameraFeature_id_t feature = (cameraFeature_id_t)YARP_FEATURE_GAIN;
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
	cameraFeature_id_t feature = (cameraFeature_id_t)YARP_FEATURE_EXPOSURE;
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
	cameraFeature_id_t feature = (cameraFeature_id_t)YARP_FEATURE_BRIGHTNESS;
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

void MainWindow::on_redGainSlider_sliderReleased()
{
	cameraFeature_id_t feature = (cameraFeature_id_t)YARP_FEATURE_CAPTURE_SIZE;
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
	cameraFeature_id_t feature = (cameraFeature_id_t)YARP_FEATURE_MIRROR;
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
	cameraFeature_id_t feature = (cameraFeature_id_t)YARP_FEATURE_CAPTURE_QUALITY;
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