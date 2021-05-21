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


#pragma once

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/FrameGrabberInterfaces.h>
#include <yarp/dev/IPreciselyTimed.h>
#include <yarp/dev/IVisualParams.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Network.h>
#include <yarp/os/Property.h>
#include <yarp/os/Semaphore.h>
#include <yarp/os/Stamp.h>
#include <yarp/sig/Image.h>

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

   public:
	MainWindow(const std::string& remotePort,QWidget *parent = nullptr);
	~MainWindow();

private slots:

    void on_gainSlider_sliderReleased();


    void on_exposureSlider_sliderReleased();

    void on_brightnessSlider_sliderReleased();

    void on_redGainSlider_sliderReleased();

    void on_blueGainSlider_sliderReleased();

    void on_greenGainSlider_sliderReleased();

private:
	Ui::MainWindow *ui;

	bool readAndShowValues();

    yarp::os::Network yarp;
    yarp::dev::PolyDriver device_;
	bool initYarp(const std::string& remotePort);
	yarp::dev::IFrameGrabberControls *grabber_;

    void emitError(const std::string& text);
};
