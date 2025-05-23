#include "BPlusTree.h"

// Реализация разделения узла
template <typename T>
void BPlusTree<T>::splitChild(Node* parent, int index, Node* child) {
    // Создание нового узла
    Node* newChild = new Node(child->isLeaf);
    
    // В массив потомков родителя вставляем ссылку на нового потомка
    parent->children.insert(
        parent->children.begin() + index + 1,
        newChild);
    
    // В родителя отправляем ключ посередине
    parent->keys.insert(
        parent->keys.begin() + index, 
        child->keys[t - 1]);

    // В новый потомок перекидываем половину ключей
    newChild->keys.assign(child->keys.begin() + t, 
        child->keys.end());
    
    // В старом потомке оставляем t-1 ключей
    child->keys.resize(t - 1);

    // Если потомок является узлом, то перекидываем еще и потомков
    if (!child->isLeaf) {
        newChild->children.assign(
            child->children.begin() + t,
            child->children.end());

        child->children.resize(t);
    } else { // Если потомок - лист, то записываем его в связный список
        newChild->next = child->next;
        child->next = newChild;
    }
}

// Вставка ключа в неполный узел
template <typename T>
void BPlusTree<T>::insertNonFull(Node* node, T key)
{   // Если узел лист, то просто вставляем в отсортированную позицию
    if (node->isLeaf) {
        node->keys.insert(
            upper_bound(node->keys.begin(),node->keys.end(), key),
            key);
    }
    else {
        // Поиск подходящего потомка
        int i = node->keys.size() - 1;
        while (i >= 0 && key < node->keys[i]) {
            i--;
        }
        i++;
        // Проверка на переполненность
        if (node->children[i]->keys.size() == 2 * t - 1) {
            splitChild(node, i, node->children[i]);
            if (key > node->keys[i]) {
                i++;
            }
        }
        // Рекурсивно вызываем для вставки в потомка
        insertNonFull(node->children[i], key);
    }
}

// удаление
template <typename T>
void BPlusTree<T>::remove(Node* node, T key)
{
    // Если узел - лист находим и стираем
    if (node->isLeaf) {
        auto it = find(
            node->keys.begin(), node->keys.end(),
            key);
        if (it != node->keys.end()) {
            node->keys.erase(it);
        }
    }
    else {
        // иначе ищем индекс во внутреннем узле
        int index = lower_bound(node->keys.begin(),
            node->keys.end(), key)
            - node->keys.begin();
        // Если ключ найден во внтуреннем узле
        if (index < node->keys.size()
            && node->keys[index] == key) {
            if (node->children[index]->keys.size() >= t) {
                Node* predNode = node->children[index];
                while (!predNode->isLeaf) {
                    predNode = predNode->children.back();
                }
                T pred = predNode->keys.back();
                node->keys[index] = pred;
                remove(node->children[index], pred);
            }
            else if (node->children[index + 1]->keys.size()
                >= t) {
                Node* succNode = node->children[index + 1];
                while (!succNode->isLeaf) {
                    succNode = succNode->children.front();
                }
                T succ = succNode->keys.front();
                node->keys[index] = succ;
                remove(node->children[index + 1], succ);
            }
            else {
                merge(node, index);
                remove(node->children[index], key);
            }
        }
        else {
            if (node->children[index]->keys.size() < t) {
                if (index > 0
                    && node->children[index - 1]->keys.size()
                    >= t) {
                    borrowFromPrev(node, index);
                }
                else if (index < node->children.size() - 1
                    && node->children[index + 1]
                    ->keys.size()
                    >= t) {
                    borrowFromNext(node, index);
                }
                else {
                    if (index < node->children.size() - 1) {
                        merge(node, index);
                    }
                    else {
                        merge(node, index - 1);
                    }
                }
            }
            remove(node->children[index], key);
        }
    }
}

// получение ссылки от родителя
template <typename T>
void BPlusTree<T>::borrowFromPrev(Node* node, int index)
{
    Node* child = node->children[index];
    Node* sibling = node->children[index - 1];

    child->keys.insert(child->keys.begin(),
        node->keys[index - 1]);
    node->keys[index - 1] = sibling->keys.back();
    sibling->keys.pop_back();

    if (!child->isLeaf) {
        child->children.insert(child->children.begin(),
            sibling->children.back());
        sibling->children.pop_back();
    }
}

// функция получения ключей от дочернего
template <typename T>
void BPlusTree<T>::borrowFromNext(Node* node, int index)
{
    Node* child = node->children[index];
    Node* sibling = node->children[index + 1];

    child->keys.push_back(node->keys[index]);
    node->keys[index] = sibling->keys.front();
    sibling->keys.erase(sibling->keys.begin());

    if (!child->isLeaf) {
        child->children.push_back(
            sibling->children.front());
        sibling->children.erase(sibling->children.begin());
    }
}

// мердж функция
template <typename T>
void BPlusTree<T>::merge(Node* node, int index)
{
    Node* child = node->children[index];
    Node* sibling = node->children[index + 1];

    child->keys.push_back(node->keys[index]);
    child->keys.insert(child->keys.end(),
        sibling->keys.begin(),
        sibling->keys.end());
    if (!child->isLeaf) {
        child->children.insert(child->children.end(),
            sibling->children.begin(),
            sibling->children.end());
    }

    node->keys.erase(node->keys.begin() + index);
    node->children.erase(node->children.begin() + index
        + 1);

    delete sibling;
}

// Вывод начиная с какого-то узла
template <typename T>
void BPlusTree<T>::printTree(Node* node, int level)
{
    if (node != nullptr) {
        for (int i = 0; i < level; ++i) {
            cout << "  ";
        }
        for (const T& key : node->keys) {
            cout << key << "\t";
        }
        cout << endl;
        for (Node* child : node->children) {
            printTree(child, level + 1);
        }
    }
}

// вывод дерева (рекурсивно)
template <typename T> void BPlusTree<T>::printTree()
{
    printTree(root, 0);
}

// Поиск
template <typename T> bool BPlusTree<T>::search(T key)
{
    Node* current = root;
    while (current != nullptr) {
        int i = 0;
        while (i < current->keys.size()
            && key > current->keys[i]) {
            i++;
        }
        if (i < current->keys.size()
            && key == current->keys[i]) {
            return true;
        }
        if (current->isLeaf) {
            return false;
        }
        current = current->children[i];
    }
    return false;
}

// запрос диапозона
template <typename T>
vector<T> BPlusTree<T>::rangeQuery(T lower, T upper)
{
    vector<T> result;
    Node* current = root;
    while (!current->isLeaf) {
        int i = 0;
        while (i < current->keys.size()
            && lower > current->keys[i]) {
            i++;
        }
        current = current->children[i];
    }
    while (current != nullptr) {
        for (const T& key : current->keys) {
            if (key >= lower && key <= upper) {
                result.push_back(key);
            }
            if (key > upper) {
                return result;
            }
        }
        current = current->next;
    }
    return result;
}

// вставка
template <typename T> void BPlusTree<T>::insert(T key)
{   // Если корня нет
    if (root == nullptr) {
        root = new Node(true); // Создаем узел, который становится листом (isLeaf=true)
        root->keys.push_back(key); // 
    }
    else {
        if (root->keys.size() == 2 * t - 1) {
            Node* newRoot = new Node();
            newRoot->children.push_back(root);
            splitChild(newRoot, 0, root);
            root = newRoot;
        }
        insertNonFull(root, key);
    }
}

// удаление
template <typename T> void BPlusTree<T>::remove(T key)
{
    if (root == nullptr) {
        return;
    }
    remove(root, key);
    if (root->keys.empty() && !root->isLeaf) {
        Node* tmp = root;
        root = root->children[0];
        delete tmp;
    }
}

// Функция для теста работы b+ дерева
int main()
{
    BPlusTree<int> tree(3);

    // Вставка
    tree.insert(10);
    tree.insert(20);
    tree.insert(5);
    tree.insert(15);
    tree.insert(25);
    tree.insert(30);
    tree.insert(21);
    tree.insert(26);

    cout << "B+ после вставки:" << endl;
    tree.printTree();

    // Поиск по ключу
    int searchKey = 15;
    cout << "\nПоиск по ключу " << searchKey << ": "
        << (tree.search(searchKey) ? "Found" : "Not Found")
        << endl;

    // запрос диапозона ключей
    int lower = 10, upper = 25;
    vector<int> rangeResult = tree.rangeQuery(lower, upper);
    cout << "\nRange query [" << lower << ", " << upper
        << "]: ";
    for (int key : rangeResult) {
        cout << key << " ";
    }
    cout << endl;

    // Удаление Ключа
    int removeKey = 20;
    tree.remove(removeKey);
    cout << "\nB+ Tree after removing " << removeKey << ":"
        << endl;
    tree.printTree();

    return 0;
}
