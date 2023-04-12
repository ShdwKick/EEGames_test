#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <random>
#include <chrono>
#include <windows.h>
#include <string>

using namespace std;

//для генерации
random_device rd;
mt19937 gen(rd());

// Структура для хранения информации об узле
struct Node
{
    string name; // Имя узла
    set<Node*> neighbors; // Соседи узла
    set<Node*> subscribers; // Подписчики на события узла
    map<Node*, int> received_data; // Данные, полученные от соседей
    int event_data; // Данные, созданные узлом
    bool destroyed; // Флаг, указывающий на то, что узел был уничтожен
    string defaulNodeName = "Node";

    // Конструктор узла
    Node(const string& name) : name(name), event_data(0), destroyed(false)
    {
        Create_Event();
    }

    // Деструктор узла
    ~Node()
    {
        // Отписываемся от всех событий
        for (auto neighbor : neighbors)
        {
            neighbor->subscribers.erase(this);
        }
    }

    // Создание события
    void Create_Event()
    {
        try
        {
            // Генерируем случайное число
            uniform_int_distribution<> dis(1, 100);
            event_data = dis(gen);
            cout << name << " created event with data " << event_data << endl;
        }
        catch (const exception& ex)
        {
            cout << "Create event error\n";
        }
    }

    // Подписка на событие
    void Subscribe(Node* neighbor)
    {
        if (neighbor == nullptr)
        {
            return;
        }
        // Проверяем, что сосед не является текущим узлом и еще не подписан на события данного узла
        if (neighbor != this && subscribers.find(neighbor) == subscribers.end())
        {
            // Добавляем соседа в список подписчиков
            subscribers.insert(neighbor);
            // Выбираем случайный обработчик события
            uniform_int_distribution<> dis(1, 2);
            int handler = dis(gen);
            // Регистрируем обработчик для события
            if (handler == 1)
            {
                cout << name << " subscribed to " << neighbor->name << " with handler S" << endl;
            }
            else {
                cout << name << " subscribed to " << neighbor->name << " with handler N" << endl;
            }
        }
    }

    // Отписка от события
    void Unsubscribe(Node* neighbor)
    {
        if (neighbor == nullptr)
        {
            return;
        }
        // Удаляем соседа из списка подписчиков
        subscribers.erase(neighbor);
        cout << name << " unsubscribed from " << neighbor->name << endl;
    }

    // Создание нового узла
    Node* CreateNode()
    {
        // Создаем новый узел
        Node* new_node = new Node(defaulNodeName + to_string(rand() % 100));
        // Добавляем новый узел в список соседей
        neighbors.insert(new_node);
        // Подписываемся на новый узел
        new_node->Subscribe(this);
        cout << name << " created new node " << new_node->name << endl;
        return new_node;
    }
    // Обработка событий
    void HandleEvents()
    {
        // Обрабатываем события от соседей
        for (auto neighbor : neighbors)
        {
            // Проверяем, что сосед не был уничтожен
            if (!neighbor->destroyed)
            {
                // Обрабатываем все события от данного соседа
                if (neighbor->received_data.size() > 0)
                {
                    for (auto& data : neighbor->received_data)
                    {
                        // Добавляем данные в список полученных данных
                        received_data[neighbor] += data.second;
                    }
                    // Очищаем список полученных данных у соседа
                    neighbor->received_data.clear();
                    // Выбираем случайный обработчик события
                    uniform_int_distribution<> dis(1, 2);
                    int handler = dis(gen);
                    // Выполняем обработчик для события
                    if (handler == 1)
                    {
                        cout << neighbor->name << " -> " << name << ": S = " << received_data[neighbor] << std::endl;
                    }
                    else {
                        cout << neighbor->name << " -> " << name << ": N = " << received_data.size() << std::endl;
                    }
                }
            }
        }
        // Обрабатываем событие, созданное данным узлом
        if (event_data > 0)
        {
            // Отправляем событие всем подписчикам
            for (auto subscriber : subscribers)
            {
                subscriber->received_data[this] += event_data;
            }
            // Очищаем данные события
            event_data = 0;
        }
    }

    Node* get()
    {
        return this;
    }
};

// Функция для обновления сети
void UpdateNetwork(vector<Node>& nodes) {
    // Обновляем каждый узел в сети 
    for (auto node : nodes)
    {
        // Проверяем, что узел не был уничтожен 
        if (!node.destroyed)
        { // Выбираем случайное действие для узла 
            uniform_int_distribution<> dis(1, 5); int action = dis(gen); // Выполняем выбранное действие 
            switch (action)
            {
            case 1:
                node.Create_Event();
                break;

            case 2:
                if (!node.neighbors.empty())
                {
                    uniform_int_distribution<> dis(0, node.neighbors.size() - 1);
                    int index = dis(gen);
                    auto it = node.neighbors.begin();
                    advance(it, index);
                    node.Subscribe(*it);
                }
                break;

            case 3:
                if (!node.subscribers.empty())
                {
                    uniform_int_distribution<> dis(0, node.subscribers.size() - 1);
                    int index = dis(gen);
                    auto it = node.subscribers.begin();
                    advance(it, index);
                    node.Unsubscribe(*it);
                }
                break;

            case 4:
                node.CreateNode();
                break;

            default:
                break;
            }
        }
    } // Синхронизируем сеть 
    for (auto node : nodes)
    {
        // Обрабатываем события узла 
        node.HandleEvents();
    }

    // Удаляем узлы, лишившиеся всех соседей 
    for (auto it = nodes.begin(); it != nodes.end();)
    {
        if ((it)->neighbors.empty())
        {
            it = nodes.erase(it);
        }
        else {
            ++it;
        }
    }
}

int main() {
    // Создаем начальную сеть узлов

    vector<Node> nodes;
    string defaultName = "Node";


    try
    {
        for (int i = 0; i < 10; i++)
        {
            nodes.push_back(Node(defaultName + to_string(i)));
        }

        // Соединяем узлы случайным образом 
        uniform_int_distribution<> dis(0, nodes.size() - 1);
        for (auto& node : nodes)
        {
            int num_neighbors = dis(gen);
            for (int i = 0; i < num_neighbors; i++)
            {
                int index = dis(gen);
                if (index >= 0 && index < nodes.size() && nodes[index].name != node.name)
                {
                    node.neighbors.insert(nodes[index].get());
                    nodes[index].get()->Subscribe(node.get());
                }
                else
                {
                    num_neighbors--;
                }
            }
        }

        // Запускаем обновление сети
        while (!nodes.empty())
        {
            UpdateNetwork(nodes);
            //Sleep(5000);
        }
    }
    catch (const exception& e)
    {
        cout << "Error";
        return 1;
    }

    return 0;
}