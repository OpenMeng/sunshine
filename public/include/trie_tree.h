/**
A trie_tree is defined in this file. It is used to search a specified value.
The trie_tree can allocate and free memory automatically, so you need't care of it.
You can use it like this:
	//here t_rate_special is the type of data which is stored in the tree.
	trie_tree<t_rate_special> tree;
	t_rate_special data;
	tree.insert(data);
	//search will return a pointer to the list of node with type of t_rate_special.
	t_rate_special *result = tree.search(data);

what you should know is that the t_rate_special must have a member fuction:void get_key(char *key);
aybe it likes:
	struct t_rate_special
	{
		char special_number[15 + 1] ;
		char other_conditions[10 + 1] ;
		t_rate_special* next ;

		void get_key(char* key)
		{
			if(key != 0)
			{
				strcpy(key, special_number) ;
			}
		}
	}
all members in the former example are required.
Before using it,you should sure sepcial_number has been initialed.
*/
#ifndef TRIE_NODE_H
#define TRIE_NODE_H

#include <iostream>
#include <string>

using namespace std;

namespace util
{
    template <class Rule> struct trie_node
    {
        int flag;
        trie_node *batch[10];
        Rule *data;
        void init()
        {
            int i = 0;
            flag = 0;
            for (i=0;i<10;i++)
            {
                batch[i] = 0;
            }
            data = 0;
        }
    };

    template <class Type> class pool
    {
        struct link
        {
            struct link *next;
        };
        struct block
        {
            enum {size = 8*1024*1024-sizeof(struct block *)};
            char mem[size];
            struct block *next;
        };

        unsigned int esize;
        block *blocks;
        link *head;

        pool(pool& p);
        void operator=(pool &p);

        void grow();
    public:
        pool();
        ~pool();
        Type *alloc();
        void free(Type *t);
    };

    template <class Type> pool<Type>::pool()
            :esize(sizeof(link)<sizeof(Type)?sizeof(Type):sizeof(link))
    {
        head = 0;
        blocks = 0;
    }

    template <class Type> pool<Type>::~pool()
    {
        block *n = blocks;
        while (n)
        {
            block *p = n;
            n = n->next;
            delete p;
        }
    }

    template <class Type> Type* pool<Type>::alloc()
    {
        if (head == 0)
        {
            grow();
        }
        link *tmp = head->next;
        Type *p = reinterpret_cast<Type *>(head);
        head = tmp;

        return p;
    }

    template <class Type> void pool<Type>::free(Type *t)
    {
        link *tmp = reinterpret_cast<link *>(t);
        tmp->next = head;
        head = tmp;
    }

    template <class Type> void pool<Type>::grow()
    {
        const int num = block::size/esize;
        block *n;
        try
        {
            n = new block;
        }
        catch (bad_alloc)
        {
            cerr<<"Out of memory"<<endl;
            exit(1);
        }
        n->next = blocks;
        blocks = n;

        char *start = n->mem;
        char *last = &start[(num-1)*esize];
        for (char *s=start;s<last;s+=esize)
        {
            reinterpret_cast<link *>(s)->next = reinterpret_cast<link *>(s+esize);
        }
        reinterpret_cast<link *>(last)->next = 0;
        head = reinterpret_cast<link *>(start);
    }

    template <class Data> class trie_tree
    {
        trie_node<Data> *root;
        pool<Data> data_pool;
        pool<trie_node<Data> > node_pool;


    public:
        trie_tree();
        //~trie_tree();
        void insert(Data data);
        Data *search(Data d);
        //void remove(trie_node<Data> *node);
    };

    template <class Data> trie_tree<Data>::trie_tree()
    {
        //dout<<endl<<"Enter trie_tree!"<<endl;
        root = 0;
        //dout<<"Exit trie_tree!"<<endl;
    }
    /*
    template <class Data> trie_tree<Data>::~trie_tree()
    {
    	//dout<<endl<<"Enter ~trie_tree!"<<endl;

    	//dout<<"Exit ~trie_tree!"<<endl;
    }
    */

    template <class Data> void  trie_tree<Data>::insert(Data data)
    {
        //dout<<endl<<"Enter insert!"<<endl;
        char key[1024];
        int i=0;
        trie_node<Data> *cur;

        if (root == 0)
        {
            root = node_pool.alloc();
            root->init();
        }

        cur = root;
        key[0] = 0;
        data.get_key(key);
        //dout<<"key = "<<key<<endl;

        while (key[i])
        {
            if (!isdigit(key[i]))
            {
                cerr<<"key must only contains char 0-9"<<endl;
                return;
            }
            i++;
        }

        i = 0;
        while (1)
        {
            //如果下一个字符不是结束符，则create一个新节点，插入到树中
            if (key[i+1] != 0)
            {
                if (cur->batch[key[i]-'0'] == 0)
                {

                    trie_node<Data> *node = node_pool.alloc();
                    node->init();
                    cur->batch[key[i]-'0'] = node;
                }
                cur = cur->batch[key[i]-'0'];
                i++;
            }
            //如果下个字符是结束符
            else
            {
                //无相应子节点，create一个，设置data
                if (cur->batch[key[i]-'0'] == 0)
                {
                    trie_node<Data> *node = node_pool.alloc();
                    node->init();
                    node->flag = 1;
                    Data *p = data_pool.alloc();
                    ;
                    //p->copy(data);
                    memcpy(p,&data,sizeof(data));
                    node->data = p;
                    p->next = 0;
                    cur->batch[key[i]-'0'] = node;
                    cur = cur->batch[key[i]-'0'];
                }
                //有子节点，更改节点的flag，将data插入链表
                else
                {
                    cur = cur->batch[key[i]-'0'];
                    cur->flag = 1;
                    Data *p = data_pool.alloc();
                    //p->copy(data);
                    memcpy(p,&data,sizeof(data));
                    p->next = cur->data;
                    cur->data = p;
                }
                break;
            }

        }
        //dout<<"Exit insert!"<<endl;
    }

    template <class Data> Data* trie_tree<Data>::search(Data data)
    {
        //dout<<endl<<"Enter search!"<<endl;
        char key[1024];
        int i = 0;
        Data *result = 0;
        trie_node<Data> *cur = root;


        if (root == 0)
        {
            return result;
        }
        key[0] = 0;
        data.get_key(key);

        //dout<<"key = "<<key<<endl;
        while (key[i]&&cur)
        {
            if (key[i]>='0'&&key[i]<='9')
            {
                if (cur->batch[key[i]-'0'] !=0)
                {
                    cur = cur->batch[key[i] - '0'];
                    if (cur->flag == 1)
                    {
                        result = cur->data;
                    }
                    i++;
                }
                else
                {
                    //dout<<"Exit search!"<<endl;
                    return result;
                }
            }
            else
            {
                break;
            }
        }
        return result;
    }

}
#endif
