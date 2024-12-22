#include "MazeStrategy.hpp"
#include <iostream>
#include <vector>

// Функция для выбора размера лабиринта
pair<int, int> chooseMazeSize() {
    vector<pair<int, int>> sizes = {
        {5, 5}, {10, 10}, {20, 20}, {30, 30}, {40, 40}, {50, 50}
    };
    
    cout << "Выберите размер лабиринта:\n";
    for (size_t i = 0; i < sizes.size(); ++i) {
        cout << i + 1 << ". " << sizes[i].first << "x" << sizes[i].second << "\n";
    }
    cout << sizes.size() + 1 << ". Ввести свой размер\n";
    
    int choice;
    cout << "Ваш выбор: ";
    cin >> choice;
    
    if (choice > 0 && choice <= static_cast<int>(sizes.size())) {
        return sizes[choice - 1];
    } else {
        int width, height;
        cout << "Введите размер лабиринта (ширина высота): ";
        cin >> width >> height;
        return {width, height};
    }
}

int main() {
    // выбор размера лабиринта
    auto [width, height] = chooseMazeSize();
    
    // cоздаем окно с подходящим размером
    const int MIN_WINDOW_SIZE = 300; // минимальная ширина и высота окна
    const int CELL_PIXELS = 30; // размер клетки в пикселях
    const int windowWidth = std::max(MIN_WINDOW_SIZE, width * CELL_PIXELS); // ширина окна
    const int windowHeight = std::max(MIN_WINDOW_SIZE, height * CELL_PIXELS); // высота окна
    const float CELL_SIZE = std::min(float(windowWidth) / width, float(windowHeight) / height); // размер клетки

    // создаем окно
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "LABIRINT");
    window.setFramerateLimit(60); // лимит кадров в секунду

    // создаем лабиринт
    Maze maze(width, height);
    
    // создаем генератор лабиринта (используем алгоритм Эллера)
    EllerMazeGenerator generator; // создаем экземпляр генератора
    generator.generate(maze); // генерируем лабиринт

    // создаем искатель пути (используем алгоритм бэктрекинга)
    BacktrackingPathFinder pathFinder; // создаем экземпляр искателя пути
    vector<pair<int, int>> path; // путь между точками (startX, startY и endX, endY)
    bool pathFound = false; // флаг наличия этого пути

    // точки для поиска пути
    int startX = -1, startY = -1;
    int endX = -1, endY = -1;
    bool startPointSelected = false; // флаг выбора начальной точки (начальная точка не может быть равна конечной)
    bool endPointSelected = false; // флаг выбора конечной точки (конечная точка не может быть равна начальной)

    cout << "\nУправление:\n";
    cout << "- Левая кнопка мыши: выбор начальной и конечной точек пути\n";
    cout << "- Правая кнопка мыши: генерация нового лабиринта\n";
    cout << "- Третий клик ЛКМ или пробел: сброс выбранных точек\n\n";

    // основной цикл программы
    while (window.isOpen()) {
        sf::Event event; // sf::Event - класс для обработки событий
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) { // если закрываем окно, то остановка программы
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed) { // если нажата кнопка мыши
                if (event.mouseButton.button == sf::Mouse::Left) { // ЛКМ
                    // преобразуем координаты мыши в координаты ячейки
                    int x = event.mouseButton.x / CELL_SIZE; // деление на размер клетки (для корректной работы)
                    int y = event.mouseButton.y / CELL_SIZE;
                    
                    if (maze.isValidCell(x, y)) { // если координаты валидны
                        if (startPointSelected && endPointSelected) { // если выбраны обе точки
                            // Третий клик - сброс точек
                            startPointSelected = false;
                            endPointSelected = false;
                            pathFound = false;
                            cout << "Точки сброшены\n\n";
                        }
                        else if (!startPointSelected) { // если не выбрана начальная точка
                            startX = x;
                            startY = y;
                            startPointSelected = true;
                            pathFound = false;
                            cout << "Начальная точка: (" << x << ", " << y << ")\n";
                        }
                        else if (!endPointSelected) { // если не выбрана конечная точка
                            endX = x;
                            endY = y;
                            endPointSelected = true;
                            cout << "Конечная точка: (" << x << ", " << y << ")\n";
                            
                            // Ищем путь
                            path = pathFinder.findPath(maze, startX, startY, endX, endY);
                            pathFound = !path.empty(); // если путь не пустой
                            if (pathFound) {
                                cout << "Путь найден!\n\n";
                            } else {
                                cout << "Путь не найден!\n\n";
                            }
                        }
                    }
                }
                else if (event.mouseButton.button == sf::Mouse::Right) { // ПКМ
                    // генерируем новый лабиринт
                    maze = Maze(width, height);
                    generator.generate(maze);
                    pathFound = false;
                    startPointSelected = false;
                    endPointSelected = false;
                    cout << "Новый лабиринт создан\n\n";
                }
            }
            else if (event.type == sf::Event::KeyPressed) { // если нажата клавиша
                if (event.key.code == sf::Keyboard::Space) { // пробел
                    // cброс выбранных точек
                    startPointSelected = false;
                    endPointSelected = false;
                    pathFound = false;
                    cout << "\nТочки сброшены\n\n";
                }
            }
        }

        // очищаем окно
        window.clear(sf::Color::Black);

        // отрисовываем лабиринт
        maze.draw(window, CELL_SIZE);

        // Отрисовываем выбранные точки и путь
        if (startPointSelected) {
            // создаем синюю точкуу
            sf::CircleShape startPoint(CELL_SIZE / 4); // круг
            startPoint.setPosition(startX * CELL_SIZE + CELL_SIZE/4, startY * CELL_SIZE + CELL_SIZE/4); // позицию
            startPoint.setFillColor(sf::Color::Blue); // цвет
            window.draw(startPoint); // отрисовываем
        }
        
        if (endPointSelected) {
            // создаем красную точку
            sf::CircleShape endPoint(CELL_SIZE / 4);
            endPoint.setPosition(endX * CELL_SIZE + CELL_SIZE/4, endY * CELL_SIZE + CELL_SIZE/4);
            endPoint.setFillColor(sf::Color::Red);
            window.draw(endPoint);
        }

        // если путь найден, отрисовываем его
        if (pathFound) {
            pathFinder.drawPath(window, CELL_SIZE, path);
        }

        // отображаем все нарисованное в окне
        window.display();
    }

    return 0;
}
