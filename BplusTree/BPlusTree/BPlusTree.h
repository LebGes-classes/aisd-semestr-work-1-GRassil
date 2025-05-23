#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

template <typename T> class BPlusTree {
public:
    // Узел в дереве
    struct Node {
        bool isLeaf; // если лист, то true
        vector<T> keys;// динамический массив ключей
        vector<Node*> children; // динамический массив детей у данного узла
        Node* next; // если лист
        
        // конструктор узла
        Node(bool leaf = false):isLeaf(leaf), next(nullptr)
        {}
    };

    Node* root; // корневой узел
    
    int t; // минимальная степень - определяет диапозон количества ключей (до 2t-1)

    // Разделение дочернего узла (при добавлении, перебалансировке)
    void splitChild(Node* parent, int index, Node* child);

    // Вставка ключа в неполный узел
    void insertNonFull(Node* node, T key);

    // Удаление ключа из узла
    void remove(Node* node, T key);

    // Взятие ключа от предыдущего родственника
    void borrowFromPrev(Node* node, int index);

    // Взятие ключа от следуюшего родственника
    void borrowFromNext(Node* node, int index);

    // Функция для соединения 2 узлов
    void merge(Node* node, int index);

    // Функция для вывода дерева
    void printTree(Node* node, int level);

public:
    BPlusTree(int degree) : root(nullptr), t(degree) {}

    void insert(T key);// вставка
    bool search(T key);// поиск
    void remove(T key);// удаление
    vector<T> rangeQuery(T lower, T upper);// диапозонный запрос (сколько значений от lower до upper)
    void printTree();//вывод дерева
};