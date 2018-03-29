#if !defined(__DML_H__)
#define __DML_H__

#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <cstring>
#include "database.h"
#include "user_exception.h"
#include "array_list.h"
#include "user_assert.h"

namespace util
{

using std::string;
using std::list;
using std::vector;
using std::set;
using std::cout;
using util::database;
using util::bad_db;
using util::bad_param;

enum data_type
{
    INT = oracle::occi::OCCIINT,
    FLOAT = oracle::occi::OCCIFLOAT,
    VARCHAR = oracle::occi::OCCI_SQLT_STR
};

// If update is executed, the follow conditions must be fit.
// It need class F provide 3 member variabls:
// static const int field_offset[...],
// static const data_type field_type[...],
// static const int field_size[...].
template<typename T>
class dml
{
public:
    dml(database& db_)
        : db(db_)
    {
    }

    virtual ~dml()
    {
    }

    // ------------------------------------------------------------------------------------------------------
    // Below functions are for array_list.
    // ------------------------------------------------------------------------------------------------------

    // Update with data struct array_list, the statement can be update/delete/merge except select.
    template<typename F, size_t N>
    void list_update(const string& sql_stmt, array_list<T, N>& data, unsigned int commit_count = N)
    {
        assert(commit_count <= 32767);
        Statement* stmt = 0;
        char** buffer;
        int i;
        try
        {
            // Allocate temporary memory.
            buffer = new char*[sizeof(F::field_type) / sizeof(int)];
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                buffer[i] = new char[commit_count * F::field_size[i]];

            stmt = db.create_statement(sql_stmt);
            typename array_list<T, N>::iterator iter;
            for (iter = data.begin(); iter != data.end(); ++iter)
            {
                // Bind.
                for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                    stmt->setDataBuffer(i + 1, buffer[i], Type(F::field_type[i]), F::field_size[i], 0);
                // Copy to array fields, and execute.
                int execute_count;
                for (int row_off = 0; row_off < iter->size(); row_off += commit_count)
                {
                    execute_count = iter->size() - row_off;
                    if (execute_count > commit_count)
                        execute_count = commit_count;
                    // Copy.
                    for (i = 0; i < execute_count; i++)
                    {
                        for (int j = 0; j < sizeof(F::field_type) / sizeof(int); j++)
                        {
                            memcpy(buffer[j] + i * F::field_size[j],
                                reinterpret_cast<char*>(iter->begin() + i + row_off) + F::field_offset[j],
                                F::field_size[j]);
                        }
                    }
                    // Execute.
                    stmt->executeArrayUpdate(execute_count);
                }
            }
            db.terminate_statement(stmt);
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                delete[] buffer[i];
            delete[] buffer;
        }
        catch (SQLException& ex)
        {
            db.terminate_statement(stmt);
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                delete[] buffer[i];
            delete[] buffer;
            throw bad_db(__FILE__, __LINE__, 142, ex, sql_stmt);
        }
    }

    // Select with array_list struct.
    template<typename F, size_t N>
    int list_select(const string& sql_stmt, array_list<T, N>& data, unsigned int fetch_count = N)
    {
        assert(fetch_count <= 32767);
        Statement* stmt = 0;
        ResultSet* rset = 0;
        char** buffer;
        int i;
        try
        {
            // Allocate temporary memory.
            buffer = new char*[sizeof(F::field_type) / sizeof(int)];
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                buffer[i] = new char[fetch_count * F::field_size[i]];

            stmt = db.create_statement(sql_stmt);
            rset = stmt->executeQuery();
            // Bind.
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                rset->setDataBuffer(i + 1, buffer[i], Type(F::field_type[i]), F::field_size[i], 0);
            // Fetch.
            int rows_total = 0;
            T record;
            ResultSet::Status status = oracle::occi::ResultSet::DATA_AVAILABLE;
            while (status)
            {
                status = rset->next(fetch_count);
                // Get rows number.
                // In 9.2.0.6 or higher, getNumArrayRows() return the rows current fetched;
                // In 9.2.0.5 or lower, getNumArrayRows() return the rows total fetched.
                int rows_num = rset->getNumArrayRows();
                if (rows_num > rows_total) // 9.2.0.5 or lowerer
                    rows_num -= rows_total;
                rows_total += rows_num;
                // Copy.
                for (i = 0; i < rows_num; i++)
                {
                    for (int j = 0; j < sizeof(F::field_type) / sizeof(int); j++)
                    {
                        memcpy(reinterpret_cast<char*>(&record) + F::field_offset[j],
                            buffer[j] + i * F::field_size[j],
                            F::field_size[j]);
                    }
                    data.push_back(record);
                }
                if (rows_num < fetch_count)
                    break;
            }
            // Free.
            stmt->closeResultSet(rset);
            db.terminate_statement(stmt);
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                delete[] buffer[i];
            delete[] buffer;
            return rows_total;
        }
        catch (SQLException& ex)
        {
            if (rset)
                stmt->closeResultSet(rset);
            db.terminate_statement(stmt);
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                delete[] buffer[i];
            delete[] buffer;
            throw bad_db(__FILE__, __LINE__, 142, ex, sql_stmt);
        }
    }
    
    // ------------------------------------------------------------------------------------------------------
    // Below functions are for pointer struct, and the pointer is array of fields.
    // Memory may allocate internally, and must free externally.
    // ------------------------------------------------------------------------------------------------------

    // Allocate memory for pointer struct.
    template<typename F>
    void allocate(T& data, int data_size, sb2*** ind = 0)
    {
        int i;
        for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
        {
             *reinterpret_cast<char**>(reinterpret_cast<char*>(&data) + F::field_offset[i])
                = new char[F::field_size[i] * data_size];
        }
        if (ind)
        {
            *ind = new sb2*[sizeof(F::field_type) / sizeof(int)];
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
            {
                (*ind)[i] = new sb2[data_size];
                memset((*ind)[i], 0, data_size);
            }
        }
    }

    // Destroy memory for pointer struct.
    template<typename F>
    void destroy(T& data, sb2*** ind = 0)
    {
        int i;
        for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
             delete[] *reinterpret_cast<char**>(reinterpret_cast<char*>(&data) + F::field_offset[i]);
        if (ind)
        {
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                delete[] (*ind)[i];
            delete[] *ind;
            *ind = 0;
        }
    }

    // Update with pointer struct, the statement can be update/delete/merge except select.
    template<typename F>
    void pointer_update(const string& sql_stmt, T& data, int data_size, int offset = 0, sb2** ind = 0)
    {
        std::dout <<"["<<__FILE__<<"] LINE["<<__LINE__<<"] sql_stmt = [" << sql_stmt << "]" << std::endl;
        Statement* stmt = 0;
        try
        {
            assert(data_size > 0);
            stmt = db.create_statement(sql_stmt);
            // Bind.
            for (int i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
            {
                if (!ind)
                {
                    stmt->setDataBuffer(i + 1,
                        *reinterpret_cast<char**>(reinterpret_cast<char*>(&data)
                            + F::field_offset[i]) + offset * F::field_size[i],
                        Type(F::field_type[i]),
                        F::field_size[i],
                        0);
                }
                else
                {
                    stmt->setDataBuffer(i + 1,
                        *reinterpret_cast<char**>(reinterpret_cast<char*>(&data)
                            + F::field_offset[i]) + offset * F::field_size[i],
                        Type(F::field_type[i]),
                        F::field_size[i],
                        0,
                        ind[i] + offset);
                }
            }
            // Execute.
            stmt->executeArrayUpdate(data_size);
            db.terminate_statement(stmt);
        }
        catch (SQLException& ex)
        {
            db.terminate_statement(stmt);
            throw bad_db(__FILE__, __LINE__, 142, ex, sql_stmt);
        }
    }

    // Update with pointer struct, the statement can be update/delete/merge except select.
    template<typename F>
    void pointer_update(Statement* stmt, T& data, int data_size, int offset = 0, sb2** ind = 0)
    {
        try
        {
            assert(data_size > 0);
            // Bind.
            for (int i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
            {
                if (!ind)
                {
                    stmt->setDataBuffer(i + 1,
                        *reinterpret_cast<char**>(reinterpret_cast<char*>(&data)
                            + F::field_offset[i]) + offset * F::field_size[i],
                        Type(F::field_type[i]),
                        F::field_size[i],
                        0);
                }
                else
                {
                    stmt->setDataBuffer(i + 1,
                        *reinterpret_cast<char**>(reinterpret_cast<char*>(&data)
                            + F::field_offset[i]) + offset * F::field_size[i],
                        Type(F::field_type[i]),
                        F::field_size[i],
                        0,
                        ind[i] + offset);
                }
            }
            // Execute.
            stmt->executeArrayUpdate(data_size);
        }
        catch (SQLException& ex)
        {
            throw bad_db(__FILE__, __LINE__, 142, ex);
        }
    }

    // Select with pointer struct.
    template<typename F>
    int pointer_select(const string& sql_stmt, T& data, int data_size, int offset = 0, sb2** ind = 0)
    {
        std::dout <<"["<<__FILE__<<"] LINE["<<__LINE__<<"]sql_stmt = [" << sql_stmt << "]" << std::endl;
        Statement* stmt = 0;
        ResultSet* rset = 0;
        try
        {
            assert(data_size > 0);
            stmt = db.create_statement(sql_stmt);
            rset = stmt->executeQuery();
            // Bind.
            for (int i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
            {
                if (!ind)
                {
                    rset->setDataBuffer(i + 1,
                        *reinterpret_cast<char**>(reinterpret_cast<char*>(&data)
                            + F::field_offset[i]) + offset * F::field_size[i],
                        Type(F::field_type[i]),
                        F::field_size[i],
                        0);
                }
                else
                {
                    rset->setDataBuffer(i + 1,
                        *reinterpret_cast<char**>(reinterpret_cast<char*>(&data)
                            + F::field_offset[i]) + offset * F::field_size[i],
                        Type(F::field_type[i]),
                        F::field_size[i],
                        0,
                        ind[i] + offset);
                }
            }
            // Fetch.
            rset->next(data_size);
            // Get rows number.
            int rows_num = rset->getNumArrayRows();
            // Free.
            stmt->closeResultSet(rset);
            db.terminate_statement(stmt);
            return rows_num;
        }
        catch (SQLException& ex)
        {
            if (rset)
                stmt->closeResultSet(rset);
            db.terminate_statement(stmt);
            throw bad_db(__FILE__, __LINE__, 142, ex, sql_stmt);
        }
    }

    // Query and bind for select with pointer struct.
    template<typename F>
    ResultSet* pointer_bind(Statement* stmt, T& data, int offset = 0, sb2** ind = 0)
    {
        ResultSet* rset = 0;
        try
        {
            rset = stmt->executeQuery();
            // Bind.
            for (int i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
            {
                if (!ind)
                {
                    rset->setDataBuffer(i + 1,
                        *reinterpret_cast<char**>(reinterpret_cast<char*>(&data)
                            + F::field_offset[i]) + offset * F::field_size[i],
                        Type(F::field_type[i]),
                        F::field_size[i],
                        0);
                }
                else
                {
                    rset->setDataBuffer(i + 1,
                        *reinterpret_cast<char**>(reinterpret_cast<char*>(&data)
                            + F::field_offset[i]) + offset * F::field_size[i],
                        Type(F::field_type[i]),
                        F::field_size[i],
                        0,
                        ind[i] + offset);
                }
            }
            return rset;
        }
        catch (SQLException& ex)
        {
            if (rset)
                stmt->closeResultSet(rset);
            throw bad_db(__FILE__, __LINE__, 142, ex);
        }
    }

    // ------------------------------------------------------------------------------------------------------
    // Below functions are for array struct, memory is allocated statically.
    // ------------------------------------------------------------------------------------------------------

    // Update with array struct, the statement can be update/delete/merge except select. 
    template<typename F>
    void array_update(const string& sql_stmt, T& data, int data_size)
    {
        Statement* stmt = 0;
        try
        {
            std::dout <<"["<<__FILE__<<"] LINE["<<__LINE__<<"]sql_stmt = [" << sql_stmt << "]" << std::endl;
            assert(data_size > 0);
            stmt = db.create_statement(sql_stmt);
            // Bind.
            for (int i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
            {
                stmt->setDataBuffer(i + 1, reinterpret_cast<char*>(&data) + F::field_offset[i],
                    Type(F::field_type[i]), F::field_size[i], 0);
            }
            // Execute.
            stmt->executeArrayUpdate(data_size);
            db.terminate_statement(stmt);
        }
        catch (SQLException& ex)
        {
            db.terminate_statement(stmt);
            throw bad_db(__FILE__, __LINE__, 142, ex, sql_stmt);
        }
    }

    // Update with array struct, the statement can be update/delete/merge except select. 
    template<typename F>
    void array_update(Statement* stmt, T& data, int data_size)
    {
        try
        {
            assert(data_size > 0);
            // Bind.
            for (int i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
            {
                stmt->setDataBuffer(i + 1, reinterpret_cast<char*>(&data) + F::field_offset[i],
                    Type(F::field_type[i]), F::field_size[i], 0);
            }
            // Execute.
            stmt->executeArrayUpdate(data_size);
        }
        catch (SQLException& ex)
        {
            throw bad_db(__FILE__, __LINE__, 142, ex);
        }
    }

    // Select with array struct.
    template<typename F>
    int array_select(const string& sql_stmt, T& data, int data_size)
    {
        std::dout <<"["<<__FILE__<<"] LINE["<<__LINE__<<"]sql_stmt = [" << sql_stmt << "]" << std::endl;
        Statement* stmt = 0;
        ResultSet* rset = 0;
        try
        {
            assert(data_size > 0);
            stmt = db.create_statement(sql_stmt);
            rset = stmt->executeQuery();
            // Bind.
            for (int i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
            {
                rset->setDataBuffer(i + 1, reinterpret_cast<char*>(&data) + F::field_offset[i],
                    Type(F::field_type[i]), F::field_size[i], 0);
            }
            // Fetch.
            rset->next(data_size);
            // Get rows number.
            int rows_num = rset->getNumArrayRows();
            // Free.
            stmt->closeResultSet(rset);
            db.terminate_statement(stmt);
            return rows_num;
        }
        catch (SQLException& ex)
        {
            if (rset)
                stmt->closeResultSet(rset);
            db.terminate_statement(stmt);
            throw bad_db(__FILE__, __LINE__, 142, ex, sql_stmt);
        }
    }

    // Query and bind for select with array struct.
    template<typename F>
    ResultSet* array_bind(Statement* stmt, T& data)
    {
        ResultSet* rset = 0;
        try
        {
            rset = stmt->executeQuery();
            // Bind.
            for (int i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
            {
                rset->setDataBuffer(i + 1, reinterpret_cast<char*>(&data) + F::field_offset[i],
                    Type(F::field_type[i]), F::field_size[i], 0);
            }
            return rset;
        }
        catch (SQLException& ex)
        {
            if (rset)
                stmt->closeResultSet(rset);
            throw bad_db(__FILE__, __LINE__, 142, ex);
        }
    }

    // ------------------------------------------------------------------------------------------------------
    // Below functions are for struct array. Memory may allocate internally and free internally.
    // ------------------------------------------------------------------------------------------------------

    // Update with struct array, the statement can be update/delete/merge except select.
    template<typename F>
    void struct_update(const string& sql_stmt, T* data, int data_size, unsigned int commit_count = 10000)
    {
        assert(commit_count <= 32767);
        Statement* stmt = 0;
        char** buffer;
        int i;
        try
        {
            assert(data_size > 0);
            // Allocate temporary memory.
            buffer = new char*[sizeof(F::field_type) / sizeof(int)];
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                buffer[i] = new char[commit_count * F::field_size[i]];

            stmt = db.create_statement(sql_stmt);
            // Bind.
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                stmt->setDataBuffer(i + 1, buffer[i], Type(F::field_type[i]), F::field_size[i], 0);
            
            int times = (data_size + commit_count - 1) / commit_count;
            for (int k = 0; k < times; k++)
            {
                // ????????е??????????С????????
                int end_size;
                if (k == times - 1)
                    end_size = data_size;
                else
                    end_size = (k + 1) * commit_count;
                // Copy.
                for (i = k * commit_count; i < end_size; i++)
                {
                    for (int j = 0; j < sizeof(F::field_type) / sizeof(int); j++)
                    {
                        memcpy(buffer[j] + i * F::field_size[j],
                            reinterpret_cast<char*>(data + i) + F::field_offset[j],
                            F::field_size[j]);
                    }
                }
                // Execute.
                if (k == times - 1)
                    stmt->executeArrayUpdate(data_size - k * commit_count);
                else
                    stmt->executeArrayUpdate(commit_count);
            }
        }
        catch (SQLException& ex)
        {
            db.terminate_statement(stmt);
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                delete[] buffer[i];
            delete[] buffer;
            throw bad_db(__FILE__, __LINE__, ex, 142, sql_stmt);
        }

        db.terminate_statement(stmt);
        for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
            delete[] buffer[i];
        delete[] buffer;
    }

    // Select with struct.
    template<typename F>
    int struct_select(const string& sql_stmt, T* data, int data_size, unsigned int fetch_count = 10000)
    {
        assert(fetch_count <= 32767);
        Statement* stmt = 0;
        ResultSet* rset = 0;
        char** buffer;
        int i;
        try
        {
            assert(data_size > 0);
            assert(fetch_count > 0);
            // Allocate temporary memory.
            buffer = new char*[sizeof(F::field_type) / sizeof(int)];
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                buffer[i] = new char[fetch_count * F::field_size[i]];

            stmt = db.create_statement(sql_stmt);
            rset = stmt->executeQuery();
            // Bind.
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                rset->setDataBuffer(i + 1, buffer[i], Type(F::field_type[i]), F::field_size[i], 0);
            // Fetch.
            int rows_total = 0;
            ResultSet::Status status = oracle::occi::ResultSet::DATA_AVAILABLE;
            while (status)
            {
                status = rset->next(fetch_count);
                // Get rows number.
                // In 9.2.0.6 or higher, getNumArrayRows() return the rows current fetched;
                // In 9.2.0.5 or lower, getNumArrayRows() return the rows total fetched.
                int rows_num = rset->getNumArrayRows();
                if (rows_num > rows_total) // 9.2.0.5 or lowerer
                    rows_num -= rows_total;
                if ( rows_total + rows_num >= data_size )
                {
                    rows_num = data_size % fetch_count;
                    if ( rows_num == 0 ) rows_num = fetch_count;
                    status = oracle::occi::ResultSet::END_OF_FETCH;
                }
                // Copy.
                for (i = 0; i < rows_num; i++)
                {
                    for (int j = 0; j < sizeof(F::field_type) / sizeof(int); j++)
                    {
                        memcpy(reinterpret_cast<char*>(data + i + rows_total) + F::field_offset[j],
                            buffer[j] + i * F::field_size[j],
                            F::field_size[j]);
                    }
                }
                rows_total += rows_num;
            }
            // Free.
            stmt->closeResultSet(rset);
            db.terminate_statement(stmt);
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                delete[] buffer[i];
            delete[] buffer;
            return rows_total;
        }
        catch (SQLException& ex)
        {
            if (rset)
                stmt->closeResultSet(rset);
            db.terminate_statement(stmt);
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                delete[] buffer[i];
            delete[] buffer;
            throw bad_db(__FILE__, __LINE__, 142, ex, sql_stmt);
        }

    }
    
    // ------------------------------------------------------------------------------------------------------
    // Below functions are for vector.
    // ------------------------------------------------------------------------------------------------------
	// update with vector.
    template<typename F>
    void vector_update(const string& sql_stmt, vector<T>& data, unsigned int commit_count)
    {
        assert(commit_count <= 32767);
        Statement* stmt = 0;
        char** buffer;
        int i;
        try
        {
            // Allocate temporary memory.
            buffer = new char*[sizeof(F::field_type) / sizeof(int)];
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                buffer[i] = new char[commit_count * F::field_size[i]];

            stmt = db.create_statement(sql_stmt);

            // Bind.
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                stmt->setDataBuffer(i + 1, buffer[i], Type(F::field_type[i]), F::field_size[i], 0);
            // Copy to array fields, and execute.
            int execute_count;
            for (int row_off = 0; row_off < data.size(); row_off += commit_count)
            {
                execute_count = data.size() - row_off;
                if (execute_count > commit_count)
                    execute_count = commit_count;
                // Copy.
                for (i = 0; i < execute_count; i++)
                {
                    for (int j = 0; j < sizeof(F::field_type) / sizeof(int); j++)
                    {
                        memcpy(buffer[j] + i * F::field_size[j],
                            reinterpret_cast<char*>(&data[i + row_off]) + F::field_offset[j],
                            F::field_size[j]);
                    }
                }
                // Execute.
                stmt->executeArrayUpdate(execute_count);
            }
            
            db.terminate_statement(stmt);
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                delete[] buffer[i];
            delete[] buffer;
        }
        catch (SQLException& ex)
        {
            db.terminate_statement(stmt);
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                delete[] buffer[i];
            delete[] buffer;
            throw bad_db(__FILE__, __LINE__, 142, ex, sql_stmt);
        }
    }
    
    
    // Select with vector.
    template<typename F>
    int vector_select(const string& sql_stmt, vector<T>& data, unsigned int fetch_count ,bool flag)
    {
        assert(fetch_count <= 32767);
        Statement* stmt = 0;
        ResultSet* rset = 0;
        char** buffer;
        int i;
        try
        {
            // Allocate temporary memory.
            buffer = new char*[sizeof(F::field_type) / sizeof(int)];
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                buffer[i] = new char[fetch_count * F::field_size[i]];

            stmt = db.create_statement(sql_stmt);
            rset = stmt->executeQuery();
            // Bind.
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                rset->setDataBuffer(i + 1, buffer[i], Type(F::field_type[i]), F::field_size[i], 0);
            // Fetch.
            int rows_total = 0;
            T record;
            ResultSet::Status status = oracle::occi::ResultSet::DATA_AVAILABLE;
            while (status)
            {
                status = rset->next(fetch_count);
                // Get rows number.
                // In 9.2.0.6 or higher, getNumArrayRows() return the rows current fetched;
                // In 9.2.0.5 or lower, getNumArrayRows() return the rows total fetched.
                int rows_num = rset->getNumArrayRows();
                if (rows_num > rows_total) // 9.2.0.5 or lowerer
                    rows_num -= rows_total;
                rows_total += rows_num;
                // Copy.
                for (i = 0; i < rows_num; i++)
                {
                    for (int j = 0; j < sizeof(F::field_type) / sizeof(int); j++)
                    {
                        memcpy(reinterpret_cast<char*>(&record) + F::field_offset[j],
                            buffer[j] + i * F::field_size[j],
                            F::field_size[j]);
                    }
                    data.push_back(record);
                }
                if(!flag) break;
                
                if (rows_num < fetch_count)
                    break;
                
            }
            // Free.
            stmt->closeResultSet(rset);
            db.terminate_statement(stmt);
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                delete[] buffer[i];
            delete[] buffer;
            return rows_total;
        }
        catch (SQLException& ex)
        {
            if (rset)
                stmt->closeResultSet(rset);
            db.terminate_statement(stmt);
            for (i = 0; i < sizeof(F::field_type) / sizeof(int); i++)
                delete[] buffer[i];
            delete[] buffer;
            throw bad_db(__FILE__, __LINE__, 142, ex, sql_stmt);
        }
    }

private:
    database& db;
};

}

#endif

