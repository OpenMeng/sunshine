#if !defined(__USER_ALGORITHM_H__)
#define __USER_ALGORITHM_H__

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "user_assert.h"

namespace util
{

using std::string;
using std::map;
using std::vector;
using std::cout;
using std::endl;

// Example of class KEY. It must provide function match_longest(int),operator[int] and size().
class K
{
public:
    // This function must be provided.
    static bool match_longest(int index)
    {
        switch (index)
        {
        case 6:
            return true;
        default:
            return false;
        }
    }

    const char* operator[](int index) const
    {
        switch (index)
        {
        case 0:
            return self_service_class;
        case 1:
            return self_region_code;
        case 2:
            return org_trm_id;
        case 3:
            return service_code;
        case 4:
            return opp_area_code;
        case 5:
            return opp_region_code;
        case 6:
            return special_number;
        default:
            assert(0);
        }
    }

    static int size()
    {
        return 7;
    }

    char self_service_class[2];
    char self_region_code[2];
    char org_trm_id[2];
    char service_code[4];
    char opp_area_code[9];
    char opp_region_code[2];
    char special_number[21];
};

// Example of class VALUE. It must provide function V(const V&).
class V
{
public:
    V()
    {
    }

    V(const V& v)
    {
        memcpy(number_type, v.number_type, sizeof(number_type));
        eff_date = v.eff_date;
        exp_date = v.exp_date;
        next = 0;
    }

    void print() const
    {
        for (const V* ptr = this; ptr; ptr = ptr->next)
            cout << ptr->number_type << endl;
    }

    const V* find(const V& value) const
    {
        for (const V* ptr = this; ptr; ptr = ptr->next)
        {
            if (ptr->eff_date < value.eff_date && ptr->exp_date > value.eff_date)
                return ptr;
        }
        return 0;
    }

    char number_type[4];
    int eff_date;
    int exp_date;
    V* next;
};

template <typename KEY, typename VALUE>
class mo_tree
{
public:
    mo_tree()
    {
        parent = 0;
        level = 0;
        sub_level = -1;
        branch = true;
        next = 0;
    }

    ~mo_tree()
    {
        typename map<string, mo_tree*>::iterator iter;
        // 如果是叶子，则next指向结果值，但mo_tree_map可能会有关键值需要继续往下找（缩位方式）
        for (iter = mo_tree_map.begin(); iter != mo_tree_map.end(); ++iter)
            delete iter->second;
        if (branch)
        {
            // 可能next指向上一层，这些就不需要再释放
            if (next && (next->level > level || next->level == level && next->sub_level > sub_level))
                delete next;
        }
        else
        {
            VALUE* pnext;
            VALUE* pprev;
            pnext = reinterpret_cast<VALUE*>(next);
            while (pnext)
            {
                pprev = pnext;
                pnext = pnext->next;
                delete pprev;
            }
        }
    }

    const VALUE* find(const KEY& key) const
    {
        const mo_tree* p = this;
        while (p && p->branch)
        {
            typename map<string, mo_tree*>::const_iterator map_iter;
            if (key.match_longest(p->level))
            {
                char buf[2];
                buf[0] = key[p->level][p->sub_level];
                buf[1] = '\0';
                map_iter = p->mo_tree_map.find(buf);
            }
            else
            {
                map_iter = p->mo_tree_map.find(key[p->level]);
            }
            if (map_iter == p->mo_tree_map.end())
                p = p->next; // 可能找下一个关键字，也可能回溯到上级（可能回溯多级）
            else
                p = map_iter->second; // 找下一个关键字
        }
        if (p)
            return reinterpret_cast<const VALUE*>(p->next);
        else
            return 0;
    }

    void insert(const KEY& key, const VALUE& value)
    {
        mo_tree* cur = this;
        for (int i = 0; i < key.size(); i++)
        {
            if (!strcmp(key[i], "*")) // 任意匹配
            {
                if (cur->next == 0) // 第一次则先分配内存
                    cur->next = new mo_tree(cur);
                cur = cur->next;
            }
            else if (key.match_longest(i))
            {
                typename map<string, mo_tree*>::iterator map_iter;
                char buf[2];
                buf[1] = '\0';
                cur->sub_level = 0;
                for (int j = 0; buf[0] = key[i][j]; j++)
                {
                    map_iter = cur->mo_tree_map.find(buf);
                    if (map_iter != cur->mo_tree_map.end()) // 存在则取对应的地址
                    {
                        cur = map_iter->second;
                    }
                    else // 不存在则先分配内存，再取对应的地址
                    {
                        mo_tree* tmp = new mo_tree(cur, 0);
                        cur->mo_tree_map[buf] = tmp;
                        cur = tmp;
                    }
                }
            }
            else
            {
                typename map<string, mo_tree*>::iterator map_iter = cur->mo_tree_map.find(key[i]);
                if (map_iter != cur->mo_tree_map.end()) // 存在则取对应的地址
                {
                    cur = map_iter->second;
                }
                else // 不存在则先分配内存，再取对应的地址
                {
                    mo_tree* tmp = new mo_tree(cur);
                    cur->mo_tree_map[key[i]] = tmp;
                    cur = tmp;
                }
            }
        }
        cur->branch = false;
        VALUE* tmp_value = new VALUE(value);
        tmp_value->next = reinterpret_cast<VALUE*>(cur->next);
        cur->next = reinterpret_cast<mo_tree*>(tmp_value);
    }

    void build_link()
    {
        vector<mo_tree*> first;
        vector<mo_tree*> second;
        first.push_back(this);
        while (1)
        {
            if (build_link(first, second))
                return;
            if (build_link(second, first))
                return;
        }
    }

private:
    mo_tree(mo_tree* parent_)
        : parent(parent_)
    {
        level = parent->level + 1;
        sub_level = -1;
        branch = true;
        next = 0;
    }

    mo_tree(mo_tree* parent_, int)
        : parent(parent_)
    {
        level = parent->level;
        sub_level = parent->sub_level + 1;
        branch = true;
        next = 0;
    }

    bool build_link(const vector<mo_tree*>& first, vector<mo_tree*>& second)
    {
        second.clear();
        typename vector<mo_tree*>::const_iterator vec_iter;
        for (vec_iter = first.begin(); vec_iter != first.end(); ++vec_iter)
        {
            typename map<string, mo_tree*>::const_iterator map_iter;
            for (map_iter = (*vec_iter)->mo_tree_map.begin(); map_iter != (*vec_iter)->mo_tree_map.end(); ++map_iter)
            {  
                if (!map_iter->second->branch) // 如果是叶子，则不需要处理
                    continue;
                second.push_back(map_iter->second);
                if (map_iter->second->next) // 有任意匹配字段，则不需要重新调整
                {
                    continue;
                }
                else
                {
                    mo_tree* p = map_iter->second;
                    while (p->parent && p == p->parent->next) // 匹配到任意字段
                        p = p->parent;
                    if (p->parent)
                        map_iter->second->next = p->parent->next;
                    else
                        map_iter->second->next = 0;
                }
            }

             // 有任意匹配字段，且是树枝，则可能需要重新调整
            if ((*vec_iter)->branch && (*vec_iter)->next && (*vec_iter)->next->parent == *vec_iter)
            {
                second.push_back((*vec_iter)->next);
                if ((*vec_iter)->next->next) // 有任意匹配字段，则不需要重新调整
                {
                    continue;
                }
                else
                {
                    mo_tree* p = *vec_iter;
                    while (p->parent && p == p->parent->next) // 匹配到任意字段
                        p = p->parent;
                    if (p->parent)
                        (*vec_iter)->next->next = p->parent->next;
                    else
                        (*vec_iter)->next->next = 0;
                }
            }
        }
        if (second.size() == 0)
            return true;
        else
            return false;
    }

    int level; // 层次，表示第几个条件
    int sub_level; // 子层次，对缩位字段，表示第几个字符
    bool branch; // 是树枝还是叶子，如果false，则next指针指向结果值
    map<string, mo_tree*> mo_tree_map; // 条件与下一条件指针对
    mo_tree* next; // 查找过程中的下一个条件（任意值）、回溯条件或结果值
    mo_tree* parent; // 父节点
};

}

#endif

