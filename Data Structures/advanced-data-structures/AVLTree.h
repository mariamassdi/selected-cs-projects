#ifndef SCRIPTS_AVLTREE_H
#define SCRIPTS_AVLTREE_H
using namespace std;



template <class T, class M>
class Tree
{
private:
int max1(int a, int b)
{
    return a > b ? a : b;
}
public:
    class Node
    {
        M key;
        T info;
        Node *left;
        Node *right;
        int height;

    public:
        Node() = default;
        Node(M key, T info) : key(key), info(info), left(nullptr), right(nullptr), height(0)
        {
        }
        M getKey()
        {
            return this->key;
        }
        Node *getLeft()
        {
            return this->left;
        }
        Node *getRight()
        {
            return this->right;
        }
        void setLeft(Node *left)
        {
            this->left = left;
        }
        void setRight(Node *right)
        {
            this->right = right;
        }
        int getHeight()
        {
            return this->height;
        }
        void setKey(M key)
        {
            this->key = key;
        }
        void setT(T info)
        {
            this->info = info;
        }
        T &getT()
        {
            return this->info;
        }
        void setHeight(int height)
        {
            this->height = height;
        }
    };
    Node *root;
    Tree() : root(nullptr)
    {
    }
    Tree(Node *root) : root(root) {}
    ~Tree()
    {
        Clear();
    }
    Tree(Tree &other)
    {
        this->root = other.root;
    }
    void Clear()
    {
        Clearaux(root);
        root = nullptr;
    }

    void Clearaux(Node *root)
    {
        if (root != nullptr)
        {
            Clearaux(root->getLeft());
            Clearaux(root->getRight());
            delete root;
        }
    }

    int Balance(Node *node)
    {
        if (node == nullptr)
        {
            return 0;
        }
        return getNodeHeight(node->getLeft()) + 1 - getNodeHeight(node->getRight()) - 1;
    }

    T *find(Node *root, M key)
    {
        if (root == nullptr)
        {
            return nullptr;
        }
        if (root->getKey() == key)
        {
            return &root->getT();
        }
        else if (root->getKey() > key)
        {
            return find(root->getLeft(), key);
        }
        return find(root->getRight(), key);
    }

    int getNodeHeight(Node *node)
    {
        return node == nullptr ? -1 : node->getHeight();
    }

    Node *insert(Node *root, M key, T info)
    {
        if (find(root, key) != nullptr)
        {
            return root;
        }
        if (root == nullptr)
        {
            return new Node(key, info);
        }
        if (key < root->getKey())
        {
            root->setLeft(insert(root->getLeft(), key, info));
        }
        else if (key > root->getKey())
        {
            root->setRight(insert(root->getRight(), key, info));
        }
        else
        {
            return root;
        }

        root->setHeight(max1(getNodeHeight(root->getLeft()), getNodeHeight(root->getRight())) + 1);
        int balance = Balance(root);
        if (balance >= 2 && key > root->getLeft()->getKey())
        {
            root->setLeft(leftRotation(root->getLeft()));
            return RightRotation(root);
        }
        else if (balance >= 2 && key < root->getLeft()->getKey())
        {
            return RightRotation(root);
        }
        else if (balance <= -2 && key > root->getRight()->getKey())
        {
            return leftRotation(root);
        }
        else if (balance <= -2 && key < root->getRight()->getKey())
        {
            root->setRight(RightRotation(root->getRight()));
            return leftRotation(root);
        }
        return root;
    }

    Node *leftRotation(Node *root)
    {
        Node *rootRight = root != nullptr ? root->getRight() : nullptr;
        Node *rootRightLeft =
            rootRight != nullptr ? rootRight->getLeft() : nullptr;
        if (rootRight != nullptr)
        {
            rootRight->setLeft(root);
        }
        if (root != nullptr)
        {
            root->setRight(rootRightLeft);
            root->setHeight(max1(getNodeHeight(root->getRight()),
                                getNodeHeight(root->getLeft())) +
                            1);
        }
        if (rootRight != nullptr)
        {
            rootRight->setHeight(max1(getNodeHeight(rootRight->getRight()),
                                     getNodeHeight(rootRight->getLeft())) +
                                 1);
        }
        return rootRight;
    }

    Node *RightRotation(Node *root)
    {
        Node *rootLeft = root != nullptr ? root->getLeft() : nullptr;
        Node *rootLeftRight = rootLeft != nullptr ? rootLeft->getRight() : nullptr;
        if (rootLeft != nullptr)
        {
            rootLeft->setRight(root);
        }
        if (root != nullptr)
        {
            root->setLeft(rootLeftRight);
            root->setHeight(max1(getNodeHeight(root->getRight()),
                                getNodeHeight(root->getLeft())) +
                            1);
        }

        if (rootLeft != nullptr)
        {
            rootLeft->setHeight(max1(getNodeHeight(rootLeft->getRight()),
                                    getNodeHeight(rootLeft->getLeft())) +
                                1);
        }
        return rootLeft;
    }

    Node *Delete(Node *root, M key)
    {
        if (root == nullptr)
        {
            return root;
        }
        if (key < root->getKey())
        {
            root->setLeft(Delete(root->getLeft(), key));
        }
        else if (key > root->getKey())
        {
            root->setRight(Delete(root->getRight(), key));
        }
        else
        {
            if (root->getLeft() != nullptr && root->getRight() != nullptr)
            {
                return deleteWithTwoChildren(root, key);
            }
            else
            {
                return deleteWithOneSonOrLess(root);
            }
        }
        root->setHeight(max1(getNodeHeight(root->getLeft()), getNodeHeight(root->getRight())) + 1);
        int balance = Balance(root);
        int leftbalance = Balance(root->getLeft());
        int rightbalance = Balance(root->getRight());

        if (balance >= 2 && leftbalance == -1) // LR
        {
            root->setLeft(leftRotation(root->getLeft()));
            return RightRotation(root);
        }
        else if (balance >= 2 && leftbalance >= 0) // LL
        {
            return RightRotation(root);
        }
        else if (balance <= -2 && rightbalance <= 0) // RR
        {
            return leftRotation(root);
        }
        else if (balance <= -2 && rightbalance == 1) // RL
        {
            root->setRight(RightRotation(root->getRight()));
            return leftRotation(root);
        }
        return root;
    }

    T *findMin(Node *root)
    {
        Node *helper = root;
        while (helper->getLeft() != nullptr)
        {
            helper = helper->getLeft();
        }
        return &helper->getT();
    }

    T *findMax(Node *root)
    {
        Node *helper = root;
        while (helper->getRight() != nullptr)
        {
            helper = helper->getRight();
        }
        return &helper->getT();
    }
    Node *deleteWithTwoChildren(Node *root, M key)
    {
        Node *helper = root->getRight();
        while (helper->getLeft() != nullptr)
        {
            helper = helper->getLeft();
        }
        swapNodes(root, helper);
        root->setRight(Delete(root->getRight(), key));
        return root;
    }

    Node *deleteWithOneSonOrLess(Node *root)
    {
        if (root->getRight() == nullptr && root->getLeft() == nullptr)
        {
            delete root;
            return nullptr;
        }
        else if (root->getRight() == nullptr)
        {
            Node *temp = root->getLeft();
            root->setRight(nullptr);
            root->setLeft(nullptr);
            delete root;
            return temp;
        }
        Node *temp = root->getRight();
        root->setRight(nullptr);
        root->setLeft(nullptr);
        delete root;
        return temp;
    }

    void swapNodes(Node *root, Node *helper)
    {
        M tempKey = helper->getKey();
        T tempT = helper->getT();
        M tempKey2 = root->getKey();
        T tempT2 = root->getT();
        helper->setKey(tempKey2);
        helper->setT(tempT2);
        root->setT(tempT);
        root->setKey(tempKey);
    }
    int treeSize()
    {
        return treeSizeHelper(this->root);
    }
    int treeSizeHelper(Node *t1)
    {
        if (t1 == nullptr)
        {
            return 0;
        }
        return 1 + treeSizeHelper(t1->getLeft()) + treeSizeHelper(t1->getRight());
    }
};

#endif // SCRIPTS_AVLTREE_H
