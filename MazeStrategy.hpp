#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <stack>

using namespace std;

// класс ячейки лабиринта, представляет собой базовый строительный блок
class Cell {
private:
    int x, y; // координаты каждой ячейки
    bool walls[4]; // массив стен: 0-верхняя, 1-правая, 2-нижняя, 3-левая
    int set; // идентификатор множества для алгоритма Эллера
    bool visited; // флаг для отслеживания посещенных ячеек при поиске пути

public:
    // конструктор ячейки, инициализирует координаты и устанавливает все стены
    Cell(int x = 0, int y = 0) : x(x), y(y), set(-1), visited(false) {  // устанавливаем координаты
        for (int i = 0; i < 4; ++i) walls[i] = true;  // устанавливаем все стены
    }
    
    // методы для работы со стенами ячейки
    void setWall(int direction, bool state) { walls[direction] = state; }
    bool hasWall(int direction) const { return walls[direction]; }
    
    // методы для работы с множеством Эллера
    void setSet(int newSet) { set = newSet; }
    int getSet() const { return set; }
    
    // методы для отслеживания посещенных ячеек
    void setVisited(bool state) { visited = state; }
    bool isVisited() const { return visited; }
    
    // получение координат ячейки
    int getX() const { return x; }
    int getY() const { return y; }
    
    // метод отрисовки ячейки в окне SFML
    void draw(sf::RenderWindow& window, float cellSize) const {
        float xPos = x * cellSize; // позиция по x (cellSize - размер клетки)
        float yPos = y * cellSize; // позиция по y
        
        // отрисовка верхней стены
        if (walls[0]) {
            sf::RectangleShape northWall(sf::Vector2f(cellSize, 2)); // создаем прямоугольник для отрисовки (sf::Vector2f(ширина, высота))
            northWall.setPosition(xPos, yPos); // устанавливаем позицию
            northWall.setFillColor(sf::Color::White); // устанавливаем цвет
            window.draw(northWall); // отрисовываем
        }
        // отрисовка правой стены
        if (walls[1]) {
            sf::RectangleShape eastWall(sf::Vector2f(2, cellSize));
            eastWall.setPosition(xPos + cellSize, yPos);
            eastWall.setFillColor(sf::Color::White);
            window.draw(eastWall);
        }
        // отрисовка нижней стены
        if (walls[2]) {
            sf::RectangleShape southWall(sf::Vector2f(cellSize, 2));
            southWall.setPosition(xPos, yPos + cellSize);
            southWall.setFillColor(sf::Color::White);
            window.draw(southWall);
        }
        // отрисовка левой стены
        if (walls[3]) {
            sf::RectangleShape westWall(sf::Vector2f(2, cellSize));
            westWall.setPosition(xPos, yPos);
            westWall.setFillColor(sf::Color::White);
            window.draw(westWall);
        }
    }
};

// базовый класс лабиринта, содержит сетку ячеек и основные методы работы с ней
class Maze {
protected:
    vector<vector<Cell>> grid; // двумерный массив ячеек лабиринта
    int width; // ширина лабиринта в ячейках
    int height; // высота лабиринта в ячейках
    
public:
    // конструктор лабиринта, создает сетку заданного размера
    Maze(int width, int height) : width(width), height(height) { // устанавливаем размер
        grid.resize(height, vector<Cell>(width)); // создаем двумерный массив
        for (int y = 0; y < height; ++y) { // заполняем ячейки
            for (int x = 0; x < width; ++x) {
                grid[y][x] = Cell(x, y); // создаем ячейку
            }
        }
    }
    
    virtual ~Maze() = default; // деструктор для освобождения памяти
    
    // проверка валидности координат ячейки
    bool isValidCell(int x, int y) const { // проверяем, что координаты в пределах лабиринта
        return x >= 0 && x < width && y >= 0 && y < height;
    }
    
    // методы доступа к ячейкам
    const Cell& getCell(int x, int y) const { return grid[y][x]; } // получение ячейки по координатам
    Cell& getCell(int x, int y) { return grid[y][x]; } // получение ячейки по координатам
    
    // получение размеров лабиринта
    int getWidth() const { return width; } // получение ширины лабиринта
    int getHeight() const { return height; } // получение высоты лабиринта
    
    // получение списка соседних ячеек для заданной позиции
    vector<pair<int, int>> getNeighbors(int x, int y) const {
        vector<pair<int, int>> neighbors; // список соседних ячейк
        const int dx[] = {0, 1, 0, -1};  // смещения по x для четырех направлений
        const int dy[] = {-1, 0, 1, 0};  // смещения по y для четырех направлений
        
        // проверяем все четыре направления
        for (int i = 0; i < 4; ++i) {
            int newX = x + dx[i]; // новые координаты
            int newY = y + dy[i];
            if (isValidCell(newX, newY)) { // если координаты в пределах лабиринта
                neighbors.push_back({newX, newY}); // добавляем соседнюю ячейку
            }
        }
        return neighbors; // возвращаем список соседних ячейк
    }
    
    // отрисовка всего лабиринта
    void draw(sf::RenderWindow& window, float cellSize) const {
        for (const auto& row : grid) {
            for (const auto& cell : row) {
                cell.draw(window, cellSize);
            }
        }
    }
};

// интерфейс для стратегий генерации лабиринта
class IMazeGenerator {
public:
    virtual ~IMazeGenerator() = default;
    virtual void generate(Maze& maze) = 0; // чисто виртуальный метод для генерации лабиринта
};

// конкретная реализация генератора лабиринта, использующая алгоритм Эллера
class EllerMazeGenerator : public IMazeGenerator {
private:
    mt19937 rng; // генератор случайных чисел
    int nextSet; // следующий доступный идентификатор множества
    
    // инициализация строки лабиринта, назначение множеств непомеченным ячейкам
    void initializeRow(Maze& maze, int row) {
        for (int x = 0; x < maze.getWidth(); ++x) { // проходим по всем ячейкам
            if (maze.getCell(x, row).getSet() == -1) { // если ячейка не помечена
                maze.getCell(x, row).setSet(nextSet++); // назначаем ей новое множество
            }
        }
    }
    
    // объединение множеств в строке случайным образом
    void mergeSets(Maze& maze, int row) { 
        uniform_int_distribution<int> dist(0, 1); // генератор случайных чисел
        
        for (int x = 0; x < maze.getWidth() - 1; ++x) { // проходим по всем ячейкам
            // случайное решение об объединении соседних ячеек
            if (dist(rng) == 1 && maze.getCell(x, row).getSet() != maze.getCell(x + 1, row).getSet()) { // если множества разные
                // убираем стену между ячейками
                maze.getCell(x, row).setWall(1, false); // убираем стену влево
                maze.getCell(x + 1, row).setWall(3, false); // убираем стену вправо
                
                // объединяем множества
                int oldSet = maze.getCell(x + 1, row).getSet(); // сохраняем старое множество
                int newSet = maze.getCell(x, row).getSet(); // новое множество
                for (int i = 0; i < maze.getWidth(); ++i) { // проходим по всем ячейкам
                    if (maze.getCell(i, row).getSet() == oldSet) { // если ячейка имеет старое множество
                        maze.getCell(i, row).setSet(newSet); // изменяем множество
                    }
                }
            }
        }
    }
    
    // добавление вертикальных соединений между строками
    void addVerticalConnections(Maze& maze, int row) {
        if (row >= maze.getHeight() - 1) return; // если это последняя строка, то ничего не делаем
        
        uniform_int_distribution<int> dist(0, 1); // генератор случайных чисел
        map<int, vector<int>> setMembers; // список ячейк в каждом множестве
        
        for (int x = 0; x < maze.getWidth(); ++x) { // проходим по всем ячейкам
            setMembers[maze.getCell(x, row).getSet()].push_back(x); // добавляем ячейку в список
        }
        
        for (const auto& pair : setMembers) { // проходим по всем множествам
            const auto& members = pair.second; // список ячейк
            bool hasConnection = false; // флаг наличия соединения
            
            for (int x : members) { // проходим по всем ячейкам
                if (!hasConnection || dist(rng) == 1) { // если соединения нет или случайное число равно 1
                    maze.getCell(x, row).setWall(2, false); // убираем верхнюю стену
                    maze.getCell(x, row + 1).setWall(0, false); // убираем нижнюю стену
                    maze.getCell(x, row + 1).setSet(maze.getCell(x, row).getSet()); // объединяем множества
                    hasConnection = true; // устанавливаем флаг
                }
            }
            
            if (!hasConnection && !members.empty()) { // если соединения нет и список не пуст
                int x = members[uniform_int_distribution<int>(0, members.size() - 1)(rng)]; // выбираем случайную ячейку
                maze.getCell(x, row).setWall(2, false); // убираем верхнюю стену
                maze.getCell(x, row + 1).setWall(0, false); // убираем нижнюю стену
                maze.getCell(x, row + 1).setSet(maze.getCell(x, row).getSet()); // объединяем множества
            }
        }
    }
    
    // обработка последней строки лабиринта
    void processLastRow(Maze& maze) {
        int lastRow = maze.getHeight() - 1; // индекс последней строки
        initializeRow(maze, lastRow); // инициализируем последнюю строку
        
        // объединяем все различные множества в последней строке
        for (int x = 0; x < maze.getWidth() - 1; ++x) { // проходим по всем ячейкам
            if (maze.getCell(x, lastRow).getSet() != maze.getCell(x + 1, lastRow).getSet()) { // если множества разные
                maze.getCell(x, lastRow).setWall(1, false); // убираем стену влево
                maze.getCell(x + 1, lastRow).setWall(3, false); // убираем стену вправо
                
                int oldSet = maze.getCell(x + 1, lastRow).getSet(); // сохраняем старое множество
                int newSet = maze.getCell(x, lastRow).getSet(); // новое множество
                for (int i = 0; i < maze.getWidth(); ++i) { // проходим по всем ячейкам
                    if (maze.getCell(i, lastRow).getSet() == oldSet) { // если множество равно старому
                        maze.getCell(i, lastRow).setSet(newSet); // объединяем множества
                    }
                }
            }
        }
    }

public:
    // конструктор, инициализирует генератор случайных чисел
    EllerMazeGenerator() : rng(random_device{}()), nextSet(1) {}
    
    // основной метод генерации лабиринта
    void generate(Maze& maze) override {
        // генерируем лабиринт построчно
        for (int row = 0; row < maze.getHeight() - 1; ++row) { // проходим по всем строкам
            initializeRow(maze, row); // инициализируем строку
            mergeSets(maze, row); // объединяем множества
            addVerticalConnections(maze, row); // добавляем вертикальные соединения
        }
        processLastRow(maze); // обрабатываем последнюю строку

        // добавляем внешние границы лабиринта
        for (int x = 0; x < maze.getWidth(); ++x) { // проходим по всем ячейкам
            maze.getCell(x, 0).setWall(0, true); // верхняя граница
            maze.getCell(x, maze.getHeight() - 1).setWall(2, true); // нижняя граница
        }
        for (int y = 0; y < maze.getHeight(); ++y) {
            maze.getCell(0, y).setWall(3, true); // левая граница
            maze.getCell(maze.getWidth() - 1, y).setWall(1, true); // правая граница
        }
    }
};

// интерфейс для стратегий поиска пути в лабиринте
class IPathFinder {
public:
    virtual ~IPathFinder() = default;
    // чисто виртуальный метод поиска пути
    virtual vector<pair<int, int>> findPath(const Maze& maze, 
        int startX, int startY, int endX, int endY) = 0;

protected:
    // вспомогательный метод для проверки возможности перемещения между ячейками
    bool isValidMove(const Maze& maze, int fromX, int fromY, int toX, int toY) const { 
        if (!maze.isValidCell(toX, toY)) return false; // проверяем валидность координат
        
        int dx = toX - fromX; // расстояние по x
        int dy = toY - fromY; // расстояние по y
        
        // определяем индекс стены в зависимости от направления движения
        int wallIndex; // индекс стены
        if (dx == 1) wallIndex = 1; // движение вправо
        else if (dx == -1) wallIndex = 3; // движение влево
        else if (dy == 1) wallIndex = 2; // движение вниз
        else wallIndex = 0; // движение вверх
        
        return !maze.getCell(fromX, fromY).hasWall(wallIndex); // проверяем наличие стены в указанном направлении
    }
};

// конкретная реализация поиска пути методом поиска в глубину с возвратом
class BacktrackingPathFinder : public IPathFinder {
public:
    // метод поиска пути от начальной до конечной точки
    vector<pair<int, int>> findPath(const Maze& maze, 
        int startX, int startY, int endX, int endY) override { 
        
        vector<pair<int, int>> path; // путь

        // матрица для отслеживания посещенных ячеек
        vector<vector<bool>> visited(maze.getHeight(), 
            vector<bool>(maze.getWidth(), false)); // по умолчанию все ячейки не посещены
        
        // стек для хранения пути
        stack<pair<int, int>> stack;
        stack.push({startX, startY}); // начальная точка
        visited[startY][startX] = true; // помечаем ячейку как посещенную
        
        // матрица для хранения предыдущих ячеек для восстановления пути
        vector<vector<pair<int, int>>> prev(maze.getHeight(), 
            vector<pair<int, int>>(maze.getWidth(), {-1, -1})); // предыдущая ячейка по умолчанию не найдена
        
        bool found = false; // флаг нахождения пути
        
        // поиск пути в глубину
        while (!stack.empty() && !found) { // пока стек не пуст и путь не найден
            auto current = stack.top(); // текущая ячейка
            stack.pop(); // удаляем ячейку из стека
            
            // если достигли конечной точки
            if (current.first == endX && current.second == endY) {
                found = true; // флаг нахождения пути
                break;
            }
            
            // проверяем все соседние ячейки
            auto neighbors = maze.getNeighbors(current.first, current.second); // список соседних ячейк
            
            for (const auto& neighbor : neighbors) { // проходим по всем соседям
                if (!visited[neighbor.second][neighbor.first] && // если ячейка не посещена
                    isValidMove(maze, current.first, current.second, // и можно переместит
                              neighbor.first, neighbor.second)) { // в эту ячейку
                    stack.push(neighbor); // добавляем ячейку в стек
                    visited[neighbor.second][neighbor.first] = true; // помечаем ячейку как посещенную
                    prev[neighbor.second][neighbor.first] = current; // запоминаем предыдущую ячейку
                }
            }
        }
        
        // восстанавливаем путь, если он найден
        if (found) {
            auto current = make_pair(endX, endY); // текущая ячейка
            while (current.first != -1 && current.second != -1) { // пока не достигли начальной ячейки
                path.push_back(current); // добавляем ячейку в путь
                current = prev[current.second][current.first]; // переходим к предыдущей ячейке
            }
            reverse(path.begin(), path.end()); // переворачиваем путь
        }
        
        return path; // возвращаем путь
    }

    // метод отрисовки найденного пути
    void drawPath(sf::RenderWindow& window, float cellSize, 
        const vector<pair<int, int>>& path) { 
        if (path.empty()) return; // если путь пуст
        
        // отрисовка линии пути
        for (size_t i = 1; i < path.size() - 1; ++i) { // проходим по всем ячейкам
            sf::RectangleShape line(sf::Vector2f(cellSize / 4, cellSize / 4)); // квадрат
            line.setPosition(path[i].first * cellSize + cellSize / 2 - cellSize / 8, // позиция
                           path[i].second * cellSize + cellSize / 2 - cellSize / 8);
            line.setFillColor(sf::Color::Green); // цвет
            window.draw(line); // отрисовка
        }
    }
};
