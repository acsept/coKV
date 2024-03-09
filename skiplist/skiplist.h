#ifndef __SKIPLIST_H__
#define __SKIPLIST_H__


#include <iostream>
//#include <cstdlib>
#include <cmath>
#include <cstring>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <fstream>



#define STORE_FILE "/home/king/share/project/store/dumpFile"

// std::mutex mmtx;
// std::string delimiter = ":";

template<typename K,typename V>
class Node{
public:
    Node();
    Node(K k,V v,int);
    ~Node();
    K get_key()const;
    V get_value() const;
    void set_value(V);
    
    Node<K,V> **forward;
    int node_level;

private:
    K key;
    V value;
};


template<typename K,typename V>
class SkipList{
public:
    SkipList(int);
    ~SkipList();
    int get_random_level();
    Node<K,V>* create_node(K,V,int);
    int insert_element(K,V);
    void display_list();
    bool search_element(K);
    std::string get_element(K);
    void delete_element(K);
    void dump_file();
    void load_file();
    void clear(Node<K,V>*);
    int size();

private:
    void get_key_value_from_string(const std::string& str,std::string* key,std::string* value);
    bool is_valid_string(const std::string& str);

private:

    int max_level;
    int skip_list_level;
    Node<K,V>* header;
    std::ofstream file_writer;
    std::ifstream file_reader;
    int element_count;
    std::mutex mmtx;
    std::string delimiter = ":";
    
};


//#include "skiplist.h"

template<typename K,typename V>
Node<K,V>::Node(const K k,const V v,int level)
:node_level(level)
,key(k)
,value(v)

{
    forward = new Node<K,V>*[level+1];
    memset(forward,0,sizeof(Node<K,V>*)*(level+1));    
}

template<typename K,typename V>
Node<K,V>::~Node(){
    delete []forward;
}


template<typename K,typename V>
K Node<K,V>::get_key() const{
    return key;
}

template<typename K,typename V>
V Node<K,V>::get_value() const{
    return value;
}

template<typename K,typename V>
void Node<K,V>::set_value(V value){
    this->value = value;
}


template<typename K,typename V>
Node<K,V>* SkipList<K,V>::create_node(const K k,const V v,int level){
    Node<K,V>* node = new Node<K,V>(k,v,level);
    return node;
}

template<typename K,typename V>
int SkipList<K,V>::insert_element(const K key,const V value){
    mmtx.lock();
    Node<K,V>* current = header;
    Node<K,V>* update[max_level+1];
    memset(update,0,sizeof(Node<K,V>*)*(max_level+1));

    for(int i = skip_list_level;i>=0;i--){
        while(current->forward[i] != nullptr && current->forward[i]->get_key() < key){
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];

    if(current != nullptr && current->get_key() == key){
        std::cout<<"key: "<<key<<",exists"<<std::endl;
        mmtx.unlock();
        return 1;
    }

    if(current == nullptr || current->get_key() != key){
        int random_level = get_random_level();
        if(random_level > skip_list_level){
            for(int i = skip_list_level+1;i<random_level+1;i++){
                update[i] = header;
            }
            skip_list_level = random_level;
        }
        Node<K,V>* inserted_node = create_node(key,value,random_level);

        for(int i = 0;i<= random_level;i++){
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        std::cout<<"Successfully inserted key:"<<key<<",value:"<<value<<std::endl;
        element_count++;
    }
    mmtx.unlock();
    return 0;
}

template<typename K,typename V>
void SkipList<K,V>::display_list(){
    for(int i = 0;i<=skip_list_level;i++){
        Node<K,V>* node = header->forward[i];
        std::cout<<"Level"<<i<<": ";
        while(node!=nullptr){
            std::cout<<node->get_key()<<":"<<node->get_value()<<";";
            node = node->forward[i];
        }
        std::cout<<std::endl;
    }
}

template<typename K,typename V>
void SkipList<K,V>::dump_file(){
    std::cout<<"dump_file-------"<<std::endl;
    file_writer.open(STORE_FILE);
    Node<K,V>* node = header->forward[0];

    while(node != nullptr){
        file_writer<<node->get_key()<<":"<<node->get_value()<<"\n";
        std::cout<<node->get_key()<<":"<<node->get_value()<<";\n";
        node = node->forward[0];
    }
    file_writer.flush();
    file_writer.close();
    return ;
}

template<typename K,typename V>
void SkipList<K,V>::load_file(){
    file_reader.open(STORE_FILE);
    std::cout<<"load_file------------"<<std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();
    while(getline(file_reader,line)){
        get_key_value_from_string(line,key,value);
        if(key->empty() || value->empty()){
            continue;
        }
        insert_element(stoi(*key),*value);
        std::cout<<"key:"<<*key<<"value:"<<*value<<std::endl;
    }
    delete key;
    delete value;
    file_reader.close();
}

template<typename K,typename V>
int SkipList<K,V>::size(){
    return element_count;
}

template<typename K,typename V>
void SkipList<K,V>::get_key_value_from_string(const std::string& str,std::string* key,std::string* value){
    if(!is_valid_string(str)){
        return;
    }
    *key = str.substr(0,str.find(delimiter));
    *value = str.substr(str.find(delimiter)+1,str.length());
}

template<typename K,typename V>
bool SkipList<K,V>::is_valid_string(const std::string& str){
    if(str.empty()){
        return false;
    }
    if(str.find(delimiter) == std::string::npos){
        return false;
    }
    return true;
}

template<typename K,typename V>
void SkipList<K,V>::delete_element(K key){
    mmtx.lock();
    Node<K,V>* current = header;
    Node<K,V>* update[max_level+1];
    memset(update,0,sizeof(Node<K,V>*)*(max_level+1));

    for(int i = skip_list_level;i>=0;i--){
        while(current->forward[i] != nullptr && current->forward[i]->get_key() < key){
            current = current->forward[i];
        }
        update[i] = current;
    }
    current = current->forward[0];
    if(current != nullptr && current->get_key() == key){
        for(int i= 0;i<=skip_list_level;i++){
            if(update[i]->forward[i] != current){
                break;
            }
            update[i]->forward[i] = current->forward[i];
        }

        while(skip_list_level > 0 && header->forward[skip_list_level] == 0){
            --skip_list_level;
        }
        std::cout<<"Successfully deleted key"<<key<<std::endl;
        delete current;
        element_count--;
    }
    mmtx.unlock();
    return;
}

template<typename K,typename V>
bool SkipList<K,V>::search_element(K key){
    std::cout<<"search element-----------"<<std::endl;
    Node<K,V>* current = header;

    for(int i = skip_list_level;i>=0;i--){
        while(current->forward[i] && current->forward[i]->get_key() < key){
            current = current->forward[i];
        }
    }
    current = current->forward[0];
    if(current && current->get_key() == key){
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout<<"Not Found Key:"<<key<<std::endl;
    return false;
}




template<typename K,typename V>
std::string SkipList<K,V>::get_element(K key){
    std::cout<<"get element-----------"<<std::endl;
    Node<K,V>* current = header;

    for(int i = skip_list_level;i>=0;i--){
        while(current->forward[i] && current->forward[i]->get_key() < key){
            current = current->forward[i];
        }
    }
    current = current->forward[0];
    if(current && current->get_key() == key){
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        
        return current->get_value();
    }
    return "";
}



template<typename K,typename V>
SkipList<K,V>::SkipList(int maxlevel)
:max_level(maxlevel)
,skip_list_level(0)
,element_count(0){
    K k;
    V v;
    header = new Node<K,V>(k,v,max_level);
}

template<typename K,typename V>
SkipList<K,V>::~SkipList(){
    if(file_writer.is_open()){
        file_writer.close();
    }
    if(file_reader.is_open()){
        file_reader.close();
    }

    if(header->forward[0] != nullptr){
        clear(header->forward[0]);
    }
    delete(header);
}

template<typename K,typename V>
void SkipList<K,V>::clear(Node<K,V>* cur){
    if(cur->forward[0] != nullptr){
        clear(cur->forward[0]);
    }
    delete(cur);
}

template<typename K,typename V>
int SkipList<K,V>::get_random_level(){
    int k = 1;
    while(rand() % 2){
        k++;
    }
    k = (k <max_level) ? k : max_level;
    return k;
}

#endif