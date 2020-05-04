#include "mainwindow.h"

#include <QPainter>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	m_ui.setupUi(this);
	m_offset = { width() / 2.0, height() / 2.0 };
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

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
	m_offset -= event->pos() - m_previousMousePosition;
	m_previousMousePosition = event->pos();

	m_image.reset(new QImage(size(), QImage::Format_RGB32));
	createJobs();
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
	m_previousMousePosition = event->pos();
}

void MainWindow::drawImage()
{
	QPainter painter(m_image.get());
	m_ui.mainView->setPixmap(QPixmap::fromImage(*m_image.get()));
}

void MainWindow::calculateMandelbrot(double heightChunkStart, double heightChunkEnd, double imageW, double imageH)
{
	QColor mapping[16];
	mapping[0].setRgb(66, 30, 15);
	mapping[1].setRgb(25, 7, 26);
	mapping[2].setRgb(9, 1, 47);
	mapping[3].setRgb(4, 4, 73);
	mapping[4].setRgb(0, 7, 100);
	mapping[5].setRgb(12, 44, 138);
	mapping[6].setRgb(24, 82, 177);
	mapping[7].setRgb(57, 125, 209);
	mapping[8].setRgb(134, 181, 229);
	mapping[9].setRgb(211, 236, 248);
	mapping[10].setRgb(241, 233, 191);
	mapping[11].setRgb(248, 201, 95);
	mapping[12].setRgb(255, 170, 0);
	mapping[13].setRgb(204, 128, 0);
	mapping[14].setRgb(153, 87, 0);
	mapping[15].setRgb(106, 52, 3);

	for(int y = heightChunkStart; y < heightChunkEnd; y++)
	{
		double y0 = (y - (imageH / 2)) + m_offset.y();
		const double cIm = (m_maxIm - y0 * m_imFactor) * m_scale;

		for (int x = 0; x < imageW; x++)
		{
			double x0 = (x - (imageW / 2)) + m_offset.x();
			const double cRe = (m_minRe + x0 * m_reFactor) * m_scale;

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
				m_image->setPixel(x, y, Qt::black);
			}
			else
			{
				//double smoothed = log2(log2(cRe * cRe + cIm * cIm) / 2);  // log_2(log_2(|p|))
				//int colorI = (int)(sqrt(n + 10 - smoothed) * 256) % 2048;
				

				m_image->setPixel(x, y, mapping[n % 16].rgb());
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
