#include <algorithm>
#include "special_travel_and_fill.h"

using namespace twg;

//-----------------------------------------------------------------------------
void TravelPoints::compute(Point_i newSize) {
	m_pmas.erase(m_pmas.begin(), m_pmas.end());
	m_pmas.reserve(newSize.x * newSize.y);
	std::vector<Rect> sq_mas;
	std::vector<Rect> sq2_mas;
	bool flag = false;

	// Помещаем все изображение как часть
	sq_mas.push_back(Rect(0, 0, newSize.x, newSize.y));
	m_pmas.push_back(Point_i(0, 0));

	while (true) {
		flag = false;
		for (auto& current : sq_mas) {
			// Алгоритм закраски работает только в обратном обходе деления прямоугольника на части. Именно поэтому большая часть информации находится справа снизу, а не слева сверху
			for (int i = 3; i >= 0; --i) {
				int x, y;
				Point_i a, b;

				if (current.x() != 0 && current.y() != 0 && current.x() != 1 && current.y() != 1) {
					// Делим на 4 квадрата
					flag = true;
					x = i % 2;
					y = i / 2;
				} else if (current.bx - current.ax > 1 && i < 2) {
					// Делим на два горизонталных квадрата
					flag = true;
					x = i % 2;
					y = 0;
				} else if (current.by - current.ay > 1 && i < 2) {
					// Делим на два вертикальных квадрата
					flag = true;
					y = i % 2;
					x = 0;
				} else {
					// Не делим на квадраты
					continue;
				}

				// Вычисляем координаты прямоугольников по x и y
				if (x == 0) {
					a.x = current.ax;
					b.x = (current.ax + current.bx) / 2;
				} else {
					a.x = (current.ax + current.bx) / 2;
					b.x = current.bx;
				}
				if (y == 0) {
					a.y = current.ay;
					b.y = (current.ay + current.by) / 2;
				} else {
					a.y = (current.ay + current.by) / 2;
					b.y = current.by;
				}

				// Помещаем полученный прямоугольник в второй массив прямоугольников
				sq2_mas.push_back(Rect(a.x, a.y, b.x, b.y));
				// Если это не нулевой прямоугольник, то с такими координатами точно не было, и помещаем такую координату в массив обхода
				if (i != 0)
					m_pmas.push_back(Point_i(sq2_mas.back().ax, sq2_mas.back().ay));
			}
		}

		if (!flag)
			break;

		sq_mas.erase(sq_mas.begin(), sq_mas.end());
		std::swap(sq_mas, sq2_mas);
	}
}

//-----------------------------------------------------------------------------
Point_i TravelPoints::get(int i) const {
	return m_pmas[i];
}

//-----------------------------------------------------------------------------
int TravelPoints::size(void) const {
	return m_pmas.size();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
std::vector<bool> FillHalfrend::m_processed(1280 * 1280);

//-----------------------------------------------------------------------------
void FillHalfrend::fill(ImageBase& img) {
	size_t size = img.width() * img.height();
	for (size_t i = 0; i < size; i++)
		m_processed[i] = 0;
	m_fill(img, Point_i(0, 0));
}

//-----------------------------------------------------------------------------
void FillHalfrend::m_fill(ImageBase& img, Point_i p) {
	m_processed[p.y * img.width() + p.x] = 1;

	int i, j;
	Color a, b, c, d;

	// Проверяем, надо ли вообще закрашивать. Это условие очень сильно оптимизирует процесс, когда изображение закрашено на большой процент
	if (p.x != img.width() - 1 && p.y != img.height() - 1 && 
		img[p + Point_i(1, 0)] != Transparent && img[p + Point_i(0, 1)] != Transparent) {
		i = p.x + 1;
		j = p.y + 1;
	} else {
		// Находим ближайший закрашенный пиксель справа
		for (i = p.x + 1; i < img.width(); i++)
			if (img[Point_i(i, p.y)] != Transparent)
				break;

		// Находим ближайший закрашенный пиксель снизу
		for (j = p.y + 1; j < img.height(); j++)
			if (img[Point_i(p.x, j)] != Transparent)
				break;

		// Присваиваем цвета пикселей на границах квадрата
		a = img[p];
		if (i == img.width())
			b = a;
		else
			b = img[Point_i(i, p.y)];

		if (j == img.height())
			c = a;
		else
			c = img[Point_i(p.x, j)];

		if (i == img.width() || j == img.height())
			d = a;
		else
			d = img[Point_i(i, j)];

		// Перебираем все пиксели в квадрате, и закрашиваем их с помощью линейной интерполяции
		for (int y = p.y; y < j; y++) {
			double posy = (y - p.y)/double(j - p.y);
			Color left = getColorBetween(posy, a, c);
			Color right = getColorBetween(posy, b, d);
			for (int x = p.x; x < i; x++) {
				double posx = (x - p.x)/double(i - p.x);
				img[Point_i(x, y)] = getColorBetween(posx, left, right);
			}
		}

		// Можно закрашивать так, скорость увеличивается, изображение становится квадратным
		/*for (int y = p.y; y < j; y++)
			for (int x = p.x; x < i; x++)
				img[Point_i(x, y)] = a;*/
	}

	// Если нижний пиксель находится в изображении и он не закрашен, то применяем рекурсию закраски к нему
	if (j < img.height() && !m_processed[p.x + j * img.width()])
		m_fill(img, Point_i(p.x, j));
	// Если правый пиксель находится в изображении и он не закрашен, то применяем рекурсию закраски к нему
	if (i < img.width() && !m_processed[i + p.y * img.width()])
		m_fill(img, Point_i(i, p.y));
}