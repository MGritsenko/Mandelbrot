#include "mainwindow.h"

#include <QPainter>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	m_ui.setupUi(this);
	m_threads.resize(m_numThreads);
}

void MainWindow::wheelEvent(QWheelEvent* event)
{
	const int delta = event->delta();
	m_scale *= qPow(0.9, delta / 120.0);

	m_image.reset(new QImage(size(), QImage::Format_RGB32));

	createJobs();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
	const auto size = event->size();
	m_image.reset(new QImage(size, QImage::Format_RGB32));

	createJobs();
}

void MainWindow::drawImage()
{
	QPainter painter(m_image.get());
	m_ui.mainView->setPixmap(QPixmap::fromImage(*m_image.get()));
}

void MainWindow::calculateMandelbrot(double heightChunkStart, double heightChunkEnd, double imageW, double imageH)
{
	for(int y = heightChunkStart; y < heightChunkEnd; y++)
	{
		const double cIm = (m_maxIm - y * m_imFactor) * m_scale;

		for (int x = 0; x < imageW; x++)
		{
			const double cRe = (m_minRe + x * m_reFactor) * m_scale;
			double zRe = cRe;
			double zIm = cIm;
			bool inside = true;

			int n = 0;
			for (n = 0; n < m_iterations; n++)
			{
				const double zRe2 = zRe * zRe;
				const double zIm2 = zIm * zIm;
				if (zRe2 + zIm2 > 4)
				{
					inside = false;
					break;
				}

				zIm = 2 * zRe * zIm + cIm;
				zRe = zRe2 - zIm2 + cRe;
			}

			if (inside)
			{
				m_image->setPixel(x, y, QColor(0, 1, 255, 255).rgb());
			}
		}
	}
}

void MainWindow::createJobs()
{
	const auto imageW = width();
	const auto imageH = height();

	m_maxIm = m_minIm + (m_maxRe - m_minRe) * imageH / imageW;
	m_reFactor = (m_maxRe - m_minRe) / (static_cast<double>(imageW) - 1);
	m_imFactor = (m_maxIm - m_minIm) / (static_cast<double>(imageH) - 1);

	const auto heightChunk = imageH / (static_cast<double>(m_numThreads) + 1);
	double start = 0;
	for (auto& thread : m_threads)
	{
		const double end = start + heightChunk;

		thread = std::thread(&MainWindow::calculateMandelbrot, this, start, end, imageW, imageH);

		start = end;
	}

	calculateMandelbrot(start, start + heightChunk, imageW, imageH);

	for (auto& thread : m_threads)
	{
		thread.join();
	}

	drawImage();
}
