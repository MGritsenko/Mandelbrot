#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"

#include <QWheelEvent>
#include <QImage>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPoint>
#include <thread>
#include <vector>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);

protected:
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;

private:
	void drawImage();
	void calculateMandelbrot(double heightChunkStart, double heightChunkEnd, double imageW, double imageH);

	void createJobs();

private:
	Ui::MainWindowClass m_ui;

	std::unique_ptr<QImage> m_image;

	const double m_minRe = -2.0;
	const double m_maxRe = 1.0;
	const double m_minIm = -1.2;
	double m_maxIm;
	double m_reFactor;
	double m_imFactor;
	double m_scale = 1.0;
	QPointF m_offset;
	const int m_numThreads = 7;
	const int m_iterations = 4000;
	std::vector<std::thread> m_threads;
	QPointF m_previousMousePosition = {};
};
