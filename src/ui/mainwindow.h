#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
#endif	// MAINWINDOW_H
