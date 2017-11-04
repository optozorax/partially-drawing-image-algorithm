/**
	В этом примере показывается как работает алгоритм обхода и закраски недогруженного\недорендеренного изображения. 
	На экране показывается: 
		процент полученного изображения.
		время получения координат обхода.
		время закраски на текущей стадии.
 */

#include <twg/twg.h>
#include <twg/window/window_events.h>
#include <sstream>
#include <iomanip>
#include <twg/image/image_drawing.h>
#include "special_travel_and_fill.h"

using namespace twg;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WindowType type(stdIcon,
		Point_i(100, 0),
		Point_i(500, 500), 
		stdMin,
		stdMax,
		L"Drawing img.bmp partially",
		WINDOW_STANDART);
	WindowBase wnd(type);

	// Считываем изображение
	ImageBase img2(Point_i(1000, 1000));
	loadFromBmp(&img2, L"img.bmp");
	wnd.setClientSize(img2.size());

	ImageBase img(Point_i(1000, 1000));
	img.resize(img2.size());
	img.clear();

	// Получаем пиксели обхода и получаем время, за которое они просчитываются
	long time = GetTickCount();
	TravelPoints trvl;
	trvl.compute(img.size());
	time = GetTickCount() - time;

	// Цикл по количеству пройденных пикселей изображения. Обходим заданный процент пикселей и закрашиваем его специальным алгоритмом и показываем результат.
	const double count = 1000;
	for (double k = 0; k <= count && !wnd.isClosed(); ++k) {
		// Повторяем 5 раз для более точных измерений
		long time2 = 0;
		long sleepTime = GetTickCount();
		for (size_t i1 = 0; i1 < 5; i1++) {
			// Обязательно!!! Очистка именно прозрачным цветом
			img.clear(Transparent);
			// Обходим заданный процент пикселей
			for (int i = 0; i < trvl.size() * k / count; ++i)
				img[trvl.get(i)] = img2[trvl.get(i)];

			// Делаем закраску и замеряем время этого
			long time21 = GetTickCount();
			FillHalfrend::fill(img);
			time21 = GetTickCount() - time21;

			time2 += time21;

			if (wnd.isClosed())
				return 0;
		}
		time2 /= 5;

		// Получаем строку с информацией
		std::wstringstream sout;
		sout.precision(3);
		sout << std::setw(18) << (k/count * 100) <<L"%\nPoints time: " << std::setw(5) << (time/1000.0) << L"s" << L"\nFill time:   " << std::setw(5) << std::setprecision(3) << (time2/1000.0) << L"s";

		// Рисуем текст и фон под него на изображение
		ImageDrawing_aa imgd(&img);
		imgd.setTextStyle(TextStyle(16, L"Consolas", TEXT_NONE));
		Polygon_d poly;
		Point_i size = imgd.getTextSize(sout.str());
		poly.array.push_back(Point_i(5, 2));
		poly.array.push_back(Point_i(size.x + 10, 2));
		poly.array.push_back(size + Point_i(10, 2));
		poly.array.push_back(Point_i(5, size.y + 2));
		imgd.setBrush(Brush(White));
		imgd.drawPolygon(poly);
		imgd.drawText(Point_i(5, 15), sout.str());

		// Рисуем полученное изображение на экран
		img.copyTo(&wnd.canvas, Point_i(0, 0), Point_i(0, 0), img.size());

		// Делаем задержку, чтобы на каждый кадр затрачивалось ровно 70 мс
		sleepTime = 70 - (GetTickCount() - sleepTime);
		if (time > 0)
			sleep(time);
	}

	wnd.waitForClose();
}