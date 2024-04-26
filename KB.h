#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#ifndef FINALPROJECT_KB_H
#define FINALPROJECT_KB_H

using namespace std;

class Knowledgebase {
    private:
    long maxQueryTime = 10;

    typedef vector<string> argumentList;

    struct predicate {
        bool negated;
        string name;
        argumentList args;

        predicate negate() {
            bool s = !negated;
            string n = name;
            argumentList a = args;
            return predicate(s, n, a);
        }

        predicate(string &s) {
            const char *t = s.c_str();

            negated = t[0] == '~';

            unsigned int index = 0;
            while (t[index++] != '(') {};

            if (negated)
                name = string(t, 1, index - 2);
            else
                name = string(t, 0, index - 1);

            unsigned int length = (unsigned int) s.length();

            for (unsigned int i = index; i < length; i++) {
                if (t[i] == ',' || t[i] == ')') {
                    string temp(t, index, i - index);
                    index = i + 1;
                    args.push_back(temp);
                }
            }
        }

        predicate(bool &s, string &n, argumentList &a) {
            negated = s;
            name = n;
            args = a;
        }

        bool operator==(const predicate &p) const {
            size_t signature1 = 17;
            signature1 = signature1 * 31 + hash<bool>()(this->negated);
            signature1 = signature1 * 31 + hash<string>()(this->name);
            for (int i = 0; i < this->args.size(); i++) {
                if (isVariable(this->args[i]))
                    signature1 = signature1 * 31 + hash<string>()("x");
                else
                    signature1 = signature1 * 31 + hash<string>()(this->args[i]);
            }

            size_t signature2 = 17;
            signature2 = signature2 * 31 + hash<bool>()(p.negated);
            signature2 = signature2 * 31 + hash<string>()(p.name);
            for (int i = 0; i < p.args.size(); i++) {
                if (isVariable(p.args[i]))
                    signature2 = signature2 * 31 + hash<string>()("x");
                else
                    signature2 = signature2 * 31 + hash<string>()(p.args[i]);
            }

            return signature1 == signature2;
        }
    };

    typedef vector<predicate> sentence;

    struct hash_predicate {
        size_t operator()(const predicate &p) const {
            size_t signature = 17;
            signature = signature * 31 + hash<bool>()(p.negated);
            signature = signature * 31 + hash<string>()(p.name);
            for (int j = 0; j < p.args.size(); j++) {
                signature = signature * 31 + hash<string>()(p.args[j]);
            }
            return signature;
        }
    };

    struct hash_sentence {
        size_t operator()(const sentence &s) const {
            int variable_count = 0;
            unordered_map<string, int> visited_variables;

            size_t signature = 0;
            for (int i = 0; i < s.size(); i++) {
                size_t temp = 17;
                temp = temp * 31 + hash<bool>()(s[i].negated);
                temp = temp * 31 + hash<string>()(s[i].name);
                for (int j = 0; j < s[i].args.size(); j++) {
                    if (isVariable(s[i].args[j])) {
                        if (visited_variables.count(s[i].args[j]) == 0) {
                            //If variable hasn't been visited before
                            //Add to visited_variables
                            variable_count++;
                            visited_variables[s[i].args[j]] = variable_count;
                        }
                        temp = temp * 31 + hash<int>()(visited_variables[s[i].args[j]]);
                    } else {
                        temp = temp * 31 + hash<string>()(s[i].args[j]);
                    }
                }
                signature ^= temp;
            }
            return signature;
        }
    };

    static bool isVariable(const string &x) {
        return islower(x[0]);
    }

    static bool isLiteral(const predicate &p) {
        for (int i = 0; i < p.args.size(); i++) {
            if (isVariable(p.args[i]))
                return false;
        }

        return true;
    }

    class CNF {
        private:
        struct node {
            node *parent, *left, *right;
            string data;

            node() : parent(nullptr), left(nullptr), right(nullptr), data("") {}
        };

        static bool isOperator(string &s) {
            return (s == "(" || s == ")" || s == "~" ||
                    s == "&" || s == "|" || s == "=>");
        }

        static int operatorPrecedence(string &op) {
            if (op == "~")
                return 4;
            if (op == "&")
                return 3;
            if (op == "|")
                return 2;
            if (op == "=>")
                return 1;
            return 0;
        }

        static node *deepCopy(node *root) {
            if (root == nullptr)
                return nullptr;

            node *temp = new node;
            temp->data = root->data;
            temp->left = deepCopy(root->left);
            if (temp->left != nullptr)
                temp->left->parent = temp;
            temp->right = deepCopy(root->right);
            if (temp->right != nullptr)
                temp->right->parent = temp;

            return temp;
        }

        string createExpressionString(node *root) {
            if (root == nullptr)
                return "";

            if (root->data == "~")
                return (root->data + " " + createExpressionString(root->left));
            else
                return (createExpressionString(root->left) + " " + root->data + " " +
                        createExpressionString(root->right));
        }

        static node *createExpressionTree(vector<string> expression) {
            stack<string> op;
            stack<node *> result;

            for (int i = 0; i < expression.size(); i++) {
                string temp = expression[i];
                if (isOperator(temp)) {
                    if (op.empty() || (temp != ")" && op.top() == "(")) {
                        op.push(temp);
                    } else if (temp == "(") {
                        op.push(temp);
                    } else if (temp == ")") {
                        while (op.top() != "(") {
                            addToStack(op.top(), result);
                            op.pop();
                        }
                        op.pop();
                    } else if (operatorPrecedence(temp) > operatorPrecedence(op.top())) {
                        op.push(temp);
                    } else if (operatorPrecedence(temp) <= operatorPrecedence(op.top())) {
                        while (!op.empty() && operatorPrecedence(temp) <= operatorPrecedence(op.top())) {
                            addToStack(op.top(), result);
                            op.pop();
                        }
                        op.push(temp);
                    }
                } else {
                    addToStack(temp, result);
                }
            }

            while (!op.empty()) {
                addToStack(op.top(), result);
                op.pop();
            }

            return result.top();
        }

        vector<string> createExpressionList(node *root) {
            vector<string> result, temp;
            if (root == nullptr)
                return result;

            temp = createExpressionList(root->left);
            if (!temp.empty())
                result.insert(result.end(), temp.begin(), temp.end());

            result.push_back(root->data);

            temp = createExpressionList(root->right);
            if (!temp.empty())
                result.insert(result.end(), temp.begin(), temp.end());

            return result;
        }

        static void deleteExpressionTree(node *root) {
            if (root == nullptr)
                return;

            deleteExpressionTree(root->left);
            deleteExpressionTree(root->right);
            delete root;
        }

        static vector<string> tokenize(string s) {
            s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
            vector<string> result;

            const char *exp = s.c_str();
            unsigned length = (unsigned) s.length();

            for (unsigned i = 0; i < length; i++) {
                if (exp[i] == '(') {
                    result.push_back("(");
                } else if (exp[i] == ')') {
                    result.push_back(")");
                } else if (exp[i] == '~') {
                    result.push_back("~");
                } else if (exp[i] == '&') {
                    result.push_back("&");
                } else if (exp[i] == '|') {
                    result.push_back("|");
                } else if (exp[i] == '=' && exp[i + 1] == '>') {
                    result.push_back("=>");
                    i++;
                } else {
                    auto j = i + 1;
                    while (exp[j] != ')') { j++; };
                    string temp(exp, i, j - i + 1);
                    result.push_back(temp);
                    i = j;
                }
            }

            return result;
        }

        static void addToStack(string &op, stack<node *> &operandStack) {
            node *root = new node;
            root->data = op;

            if (isOperator(op)) {
                if (op == "~") {
                    node *operand = operandStack.top();
                    operand->parent = root;
                    operandStack.pop();
                    root->left = operand;
                    root->right = nullptr;
                } else {
                    node *operand2 = operandStack.top();
                    operand2->parent = root;
                    operandStack.pop();
                    node *operand1 = operandStack.top();
                    operand1->parent = root;
                    operandStack.pop();
                    root->left = operand1;
                    root->right = operand2;
                }
            }
            operandStack.push(root);
        }

        static node *negate(node *root) {
            if (root == nullptr)
                return nullptr;

            if (root->left == nullptr && root->right == nullptr) {
                //If leaf node is encountered i.e operand encountered
                if (root->data[0] != '~') {
                    root->data = "~" + root->data;
                } else {
                    root->data = root->data.substr(1);
                }
            } else {
                if (root->data == "~") {
                    node *t = root;
                    if (root->parent == nullptr) {
                        root = root->left;
                        root->parent = nullptr;
                    } else {
                        if (root->parent->left == root) {
                            root->parent->left = root->left;
                        } else {
                            root->parent->right = root->left;
                        }
                        root->left->parent = root->parent;
                        root = root->left;
                    }
                    delete t;
                    return root;
                } else if (root->data == "&") {
                    root->data = "|";
                } else if (root->data == "|") {
                    root->data = "&";
                }
            }

            root->left = negate(root->left);
            root->right = negate(root->right);

            return root;
        }

        static void removeImplications(node *root) {
            if (root == nullptr)
                return;
            removeImplications(root->left);
            removeImplications(root->right);
            if (root->data == "=>") {
                //A => B ------> ~A | B
                negate(root->left);
                root->data = "|";
            }
        }

        static node *resolveNegations(node *root) {
            if (root == nullptr)
                return nullptr;

            while (root->data == "~") {
                root = negate(root);
                root = negate(root);
            }

            root->left = resolveNegations(root->left);
            root->right = resolveNegations(root->right);

            return root;
        }

        static node *distribute(node *parent, node *child) {
            node *grandparent = parent->parent;
            node *leftBranch1, *leftBranch2, *rightBranch1, *rightBranch2;
            bool isParentLeftOfGrandparent = (grandparent != nullptr) ? (grandparent->left == parent) : false;

            if (parent->left == child) {
                leftBranch1 = child->left;
                leftBranch2 = child->right;
                rightBranch1 = parent->right;
                rightBranch2 = deepCopy(parent->right);
            } else {
                leftBranch1 = parent->left;
                leftBranch2 = deepCopy(parent->left);
                rightBranch1 = child->left;
                rightBranch2 = child->right;
            }

            delete parent;

            node *leftNode = new node;
            leftNode->data = "|";
            leftNode->left = leftBranch1;
            leftNode->right = rightBranch1;
            leftNode->parent = child;

            node *rightNode = new node;
            rightNode->data = "|";
            rightNode->left = leftBranch2;
            rightNode->right = rightBranch2;
            rightNode->parent = child;

            leftBranch1->parent = leftNode;
            rightBranch1->parent = leftNode;

            leftBranch2->parent = rightNode;
            rightBranch2->parent = rightNode;

            child->left = leftNode;
            child->right = rightNode;

            child->parent = grandparent;
            if (grandparent != nullptr) {
                if (isParentLeftOfGrandparent) {
                    grandparent->left = child;
                } else {
                    grandparent->right = child;
                }
            }
            return child;
        }

        static node *distributeOrOverAnd(node *root) {
            if (root == nullptr)
                return nullptr;

            if (!isOperator(root->data))
                return root;

            if (root->data == "|") {
                bool distributed = false;
                if (root->left->data == "&") {
                    root = distribute(root, root->left);
                    distributed = true;
                }

                if (root->right->data == "&") {
                    //distribute | over & on the right child
                    root = distribute(root, root->right);
                    distributed = true;
                }

                if (distributed) {
                    if (root->parent == nullptr) {
                        return distributeOrOverAnd(root);
                    } else {
                        return root;
                    }
                }
            }

            node *left = root->left;
            node *right = root->right;

            root->left = distributeOrOverAnd(root->left);
            if (left != root->left) {
                return distributeOrOverAnd(root);
            }
            root->right = distributeOrOverAnd(root->right);
            if (right != root->right) {
                return distributeOrOverAnd(root);
            }

            return root;
        }

        static vector<node *> splitSentenceOverAnd(node *root) {
            vector<node *> result, temp;
            if (root == nullptr)
                return result;

            if (root->data != "&") {
                result.push_back(root);
                return result;
            }

            temp = splitSentenceOverAnd(root->left);
            if (!temp.empty()) {
                result.insert(result.end(), temp.begin(), temp.end());
            }
            temp = splitSentenceOverAnd(root->right);
            if (!temp.empty()) {
                result.insert(result.end(), temp.begin(), temp.end());
            }

            return result;
        }

        static sentence createCNFSentence(node *root) {
            sentence result, temp;
            if (root->left == nullptr && root->right == nullptr) {
                predicate t = predicate(root->data);
                result.push_back(t);
                return result;
            }

            temp = createCNFSentence(root->left);
            if (!temp.empty())
                result.insert(result.end(), temp.begin(), temp.end());

            temp = createCNFSentence(root->right);
            if (!temp.empty())
                result.insert(result.end(), temp.begin(), temp.end());

            return result;
        }

        public:
        static sentence negateCNFSentence(sentence s) {
            for (int i = 0; i < s.size(); i++) {
                s[i] = s[i].negate();
            }
            return s;
        }

        static sentence &factorize(sentence &s) {
            unordered_set<predicate, hash_predicate> visited;
            vector<int> predicate_to_remove;
            for (int i = 0; i < s.size(); i++) {
                if (visited.count(s[i]) == 0) {
                    visited.insert(s[i]);
                } else {
                    predicate_to_remove.push_back(i);
                }
            }
            for (int i = 0; i < predicate_to_remove.size(); i++) {
                s.erase(s.begin() + predicate_to_remove[i]);
            }

            return s;
        }

        static vector<sentence> convertToCNFSentences(string &s) {
            node *expressionRoot = createExpressionTree(tokenize(s));
            removeImplications(expressionRoot);
            expressionRoot = resolveNegations(expressionRoot);
            expressionRoot = distributeOrOverAnd(expressionRoot);
            vector<node *> sentences = splitSentenceOverAnd(expressionRoot);

            sentence temp;
            vector<sentence> result;
            for (int i = 0; i < sentences.size(); i++) {
                temp = createCNFSentence(sentences[i]);
                temp = factorize(temp);
                result.push_back(temp);
            }

            deleteExpressionTree(expressionRoot);

            return result;
        }
    };

    class Database {
        private:
        struct row {
            vector<pair<unsigned int, unsigned int>> positive_literals;
            vector<pair<unsigned int, unsigned int>> negative_literals;
            vector<pair<unsigned int, unsigned int>> positive_sentences;
            vector<pair<unsigned int, unsigned int>> negative_sentences;
        };

        unordered_map<char, unsigned int> variables;

        vector<sentence> data;
        unordered_map<string, row> index;

        sentence standardizeSentence(sentence &s) {
            unordered_set<char> current_variables;
            for (int i = 0; i < s.size(); i++) {
                argumentList &args = s[i].args;
                for (int j = 0; j < args.size(); j++) {
                    if (isVariable(args[j])) {
                        char var = args[j][0];
                        if (current_variables.count(var) == 0) {
                            if (variables.count(var) == 0) {
                                variables[var] = 1;

                            } else {
                                variables[var]++;
                            }
                            current_variables.insert(var);
                        }
                        args[j] = var + to_string(variables[var]);
                    }
                }
            }

            return s;
        }

        public:
        Database copy() {
            Database t;
            t.variables = this->variables;
            t.data = this->data;
            t.index = this->index;
            return t;
        }

        void store(sentence &s) {
            s = standardizeSentence(s);
            data.push_back(s);
            unsigned long long int loc = data.size() - 1;
            for (int i = 0; i < s.size(); i++) {
                if (isLiteral(s[i])) {
                    if (!s[i].negated) {
                        index[s[i].name].positive_literals.push_back(pair<unsigned int, unsigned int>(loc, i));
                    } else {
                        index[s[i].name].negative_literals.push_back(pair<unsigned int, unsigned int>(loc, i));
                    }
                } else {
                    if (!s[i].negated) {
                        index[s[i].name].positive_sentences.push_back(pair<unsigned int, unsigned int>(loc, i));
                    } else {
                        index[s[i].name].negative_sentences.push_back(pair<unsigned int, unsigned int>(loc, i));
                    }
                }
            }
        }

        vector<pair<sentence, unsigned int>> fetch(predicate &p) {
            vector<pair<sentence, unsigned int>> result;
            vector<pair<unsigned int, unsigned int>> literalIndex;
            vector<pair<unsigned int, unsigned int>> sentenceIndex;

            if (!p.negated) {
                literalIndex = index[p.name].positive_literals;
                sentenceIndex = index[p.name].positive_sentences;
            } else {
                literalIndex = index[p.name].negative_literals;
                sentenceIndex = index[p.name].negative_sentences;
            }
            for (int i = 0; i < literalIndex.size(); i++) {
                result.push_back(pair<sentence, unsigned int>(data[literalIndex[i].first], literalIndex[i].second));
            }
            for (int i = 0; i < sentenceIndex.size(); i++) {
                result.push_back(pair<sentence, unsigned int>(data[sentenceIndex[i].first], sentenceIndex[i].second));
            }

            return result;
        }

    };

    Database DB;

    argumentList &substitute(argumentList &x, unordered_map<string, string> &theta) {
        for (int i = 0; i < x.size(); i++) {
            while (theta.count(x[i]) > 0)
                x[i] = theta[x[i]];
        }
        return x;
    }

    bool unify(argumentList &x, argumentList &y, unordered_map<string, string> &theta) {
        if (x.size() != y.size())
            return false;
        for (int i = 0; i < x.size(); i++) {
            if (x[i] != y[i]) {
                if (isVariable(x[i])) {
                    theta[x[i]] = y[i];
                    x = substitute(x,theta);
                    y = substitute(y,theta);
                } else if (isVariable(y[i])) {
                    theta[y[i]] = x[i];
                    x = substitute(x,theta);
                    y = substitute(y,theta);
                } else {
                    return false;
                }
            }
        }
        return true;
    }

    public:
    void tell(string &fact) {
        //Loop through all CNF sentences and insert into KB
        vector<sentence> sentences = CNF::convertToCNFSentences(fact);
        for (int i = 0; i < sentences.size(); i++) {
            DB.store(sentences[i]);
        }
    }

    bool ask(string &query) {
        double finishTime = get_wall_time() + maxQueryTime;
        sentence alpha = CNF::convertToCNFSentences(query)[0];
        sentence notAlpha = CNF::negateCNFSentence(alpha);
        Database KB = DB.copy();
        KB.store(notAlpha);

        queue<sentence> Frontier;
        unordered_set<sentence, hash_sentence> LoopDetector;    //Prevents duplicate sentences in KB

        Frontier.push(notAlpha);

        while (!Frontier.empty()) {
            sentence currentSentence = Frontier.front();
            Frontier.pop();

            for (int i = 0; i < currentSentence.size(); i++) {

                predicate resolver = currentSentence[i].negate();
                vector<pair<sentence, unsigned int>> resolvableSentences = KB.fetch(resolver);
                for (int j = 0; j < resolvableSentences.size(); j++) {
                    unordered_map<string, string> theta;
                    if (resolvableSentences[j].first[resolvableSentences[j].second].name == currentSentence[i].name &&
                        resolvableSentences[j].first[resolvableSentences[j].second].negated !=
                        currentSentence[i].negated) {

                        argumentList x = currentSentence[i].args;
                        argumentList y = resolvableSentences[j].first[resolvableSentences[j].second].args;

                        if (unify(x, y, theta)) {
                            sentence t1 = currentSentence;
                            sentence t2 = resolvableSentences[j].first;
                            for (int k = 0; k < t1.size(); k++)
                                t1[k].args = substitute(t1[k].args, theta);
                            for (int k = 0; k < t2.size(); k++)
                                t2[k].args = substitute(t2[k].args, theta);


                            t1.erase(t1.begin() + i);
                            t2.erase(t2.begin() + resolvableSentences[j].second);

                            sentence resolvent;
                            resolvent.insert(resolvent.end(), t1.begin(), t1.end());
                            resolvent.insert(resolvent.end(), t2.begin(), t2.end());

                            resolvent = CNF::factorize(resolvent);

                            if (resolvent.empty()) {
                                DB.store(alpha);
                                return true;
                            }

                            if (LoopDetector.count(resolvent) == 0) {
                                KB.store(resolvent);
                                Frontier.push(resolvent);
                                LoopDetector.insert(resolvent);
                            }
                        }
                    }
                }
                if (get_wall_time() > finishTime) {
                    return false;
                }
            }
        }
        return false;
    }

};

#endif //FINALPROJECT_KB_H
