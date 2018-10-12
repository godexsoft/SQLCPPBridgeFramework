//
//  sb_sqlite_adapter.h
//  SQLCPPBridgeFramework
//
//  Created by Roman Makhnenko on 13/03/2016.
//  Copyright © 2016, DataArt.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//      * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//      * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//      * Neither the name of the DataArt nor the
//      names of its contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL DataArt BE LIABLE FOR ANY
//  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once
#ifndef sb_sqlite_adapter_h
#define sb_sqlite_adapter_h

#include <sqlite3.h>
#include "sb_traits.h"
#include "sb_core.h"
#include "sb_links.h"
#include "sb_value.h"

namespace sql_bridge
{
    class sqlite_adapter
    {
    public:

#pragma mark - sql_file
        
        class sql_file
        {
            typedef std::map<std::string,sqlite3_stmt*> _TStatementsMap;
        public:
            ~sql_file();
            inline sqlite_int64 last_insert_id() const {return sqlite3_last_insert_rowid(base_);}
            inline std::string const& file_name() const {return file_name_;}
            inline int err_code() const {return err_code_;}
            const sql_file& execute(std::string const&) const;
            sqlite3_stmt* operator[](std::string const& txstm) const;

            class transactions_lock
            {
            public:
                transactions_lock(sql_file const&);
                ~transactions_lock();
                inline void rollback() {rollback_ = true;}
            private:
                bool rollback_;
                sql_file const& parent_;
            };

            class no_transactions_lock
            {
            public:
                no_transactions_lock(sql_file const&) {};
                ~no_transactions_lock() {}
                inline void rollback() {}
            };

        protected:
            sql_file(std::string const& fname);
            sql_file() = delete;
            sql_file(sql_file const&) = delete;
            sql_file(sql_file&&) = delete;
        private:
            void reset_cache() const;
            sqlite3* base_;
            std::string const file_name_;
            mutable _TStatementsMap statements_cache_;
            mutable int err_code_;
        };
        
#pragma mark - sql_reader_kv
        
        class sql_reader_kv
        {
        public:
            sql_reader_kv(sql_file const& db, std::string const& table, std::string const& key);
            sql_reader_kv(sql_file const& db, std::string const& table, bool keep_order);
            sql_reader_kv(sql_reader_kv const&) = delete;
            sql_reader_kv(sql_reader_kv&&) = delete;
            ~sql_reader_kv() {};
            bool next();
            inline bool is_valid() const {return valid_;}
            template<typename T> inline typename std::enable_if<is_convertible_to_float<T>::value>::type read_value(T& v, int fld) const {v=static_cast<T>(sqlite3_column_double(state_, fld));}
            template<typename T> inline typename std::enable_if<is_convertible_to_text<T>::value>::type read_value(T& v, int fld) const {v=reinterpret_cast<const char*>(sqlite3_column_text(state_, fld));}
            template<typename T> inline typename std::enable_if<is_convertible_to_int<T>::value>::type read_value(T& v, int fld) const {v=static_cast<T>(sqlite3_column_int64(state_, fld));}
        protected:
        private:
            // data
            sqlite3_stmt* state_;
            bool valid_;
            std::string txt_statement_;
        };
        
#pragma mark - sql_inserter_kv
        
        class sql_inserter_kv
        {
        public:
            sql_inserter_kv(sql_file const& db,std::string const& table,bool kv);
            sql_inserter_kv(sql_inserter_kv const&) = delete;
            sql_inserter_kv(sql_inserter_kv&& src)
                : state_(src.state_)
                , need_step_(src.need_step_)
                , txt_statement_(src.txt_statement_)
                , fld_num_(src.fld_num_)
            {
                src.state_ = nullptr;
                src.need_step_ = false;
                src.txt_statement_.clear();
            }
            ~sql_inserter_kv();
            void next();
            inline sql_inserter_kv& bind_key(std::string const& key) {need_step_=true;sqlite3_bind_text(state_, 1, key.c_str(), (int)key.size(), SQLITE_STATIC);return *this;}
            template<typename T> inline void bind_value(T const& val) {bind_value(val,fld_num_);}
            template<typename T> inline typename std::enable_if<is_convertible_to_float<T>::value>::type bind_value(T const& val, int fld) {need_step_=true;sqlite3_bind_double(state_, fld, val);}
            template<typename T> inline typename std::enable_if<is_convertible_to_int<T>::value>::type bind_value(T const& val, int fld) {need_step_=true;sqlite3_bind_int64(state_, fld, static_cast<sqlite3_int64>(val));}
            template<typename T> inline typename std::enable_if<is_convertible_to_text<T>::value>::type bind_value(T const& val, int fld) {need_step_=true;sqlite3_bind_text(state_, fld, val.c_str(), (int)val.size(), SQLITE_STATIC);}
        protected:
        private:
            // data
            sqlite3_stmt* state_;
            bool need_step_;
            std::string txt_statement_;
            int fld_num_;
        };

#pragma mark - sql_reader
        
        class sql_reader
        {
        public:
            sql_reader(sql_file const& db, std::string const&, sql_value const&);
            sql_reader(sql_reader const&) = delete;
            sql_reader(sql_reader&&) = delete;
            ~sql_reader() {};

            bool next();
            inline bool is_valid() const {return valid_;}
            template<typename T> inline typename std::enable_if<is_convertible_to_float<T>::value>::type read_value(T& v) {v=static_cast<T>(sqlite3_column_double(statement_, fld_num_++));}
            template<typename T> inline typename std::enable_if<is_convertible_to_text<T>::value>::type read_value(T& v) {v=reinterpret_cast<const char*>(sqlite3_column_text(statement_, fld_num_++));}
            template<typename T> inline typename std::enable_if<is_convertible_to_int<T>::value>::type read_value(T& v) {v=static_cast<T>(sqlite3_column_int64(statement_, fld_num_++));}

            template<typename T> inline void bind(T const& val) {bind_value(val,1);}
            inline void bind(sql_value const& val)
            {
                switch(val.type_)
                {
                    case sql_value::e_key_type::Integer:
                        bind(val.iValue_);
                        break;
                    case sql_value::e_key_type::Real:
                        bind(val.rValue_);
                        break;
                    case sql_value::e_key_type::String:
                        bind(val.tValue_);
                        break;
                    case sql_value::e_key_type::Empty:
                        break;
                }
            }

        private:
            // data
            sqlite3_stmt* statement_;
            bool valid_;
            std::string txt_statement_;
            int fld_num_;
            // methods
            template<typename T> inline typename std::enable_if<is_convertible_to_float<T>::value>::type bind_value(T const& val, int fld) {sqlite3_bind_double(statement_, fld, val);}
            template<typename T> inline typename std::enable_if<is_convertible_to_int<T>::value>::type bind_value(T const& val, int fld) {sqlite3_bind_int64(statement_, fld, static_cast<sqlite3_int64>(val));}
            template<typename T> inline typename std::enable_if<is_convertible_to_text<T>::value>::type bind_value(T const& val, int fld) {sqlite3_bind_text(statement_, fld, val.c_str(), (int)val.size(), SQLITE_TRANSIENT);}
        };
        
#pragma mark - sql_updater
        
        class sql_updater
        {
        public:
            sql_updater(sql_file const& db, std::string const&);
            sql_updater(sql_updater const&) = delete;
            sql_updater(sql_updater&& src)
                : statement_(src.statement_)
                , need_step_(src.need_step_)
                , txt_statement_(src.txt_statement_)
                , fld_num_(src.fld_num_)
            {
                src.statement_ = nullptr;
                src.need_step_ = false;
                src.txt_statement_.clear();
            }
            ~sql_updater();
            
            inline bool empty() const {return !statement_;}
            inline void mark_as_dirty() {need_step_=true;}
            
            template<typename T> inline void bind(T const& val) {bind_value(val,fld_num_++);}
            inline void bind(sql_value const& val)
            {
                switch(val.type_)
                {
                    case sql_value::e_key_type::Integer:
                        bind(val.iValue_);
                        break;
                    case sql_value::e_key_type::Real:
                        bind(val.rValue_);
                        break;
                    case sql_value::e_key_type::String:
                        bind(val.tValue_);
                        break;
                    case sql_value::e_key_type::Empty:
                        break;
                }
            }
            bool next();
        private:
            // data
            sqlite3_stmt* statement_;
            bool need_step_;
            std::string txt_statement_;
            int fld_num_;
            // methods
            template<typename T> inline typename std::enable_if<is_convertible_to_float<T>::value>::type bind_value(T const& val, int fld) {need_step_=true;sqlite3_bind_double(statement_, fld, val);}
            template<typename T> inline typename std::enable_if<is_convertible_to_int<T>::value>::type bind_value(T const& val, int fld) {need_step_=true;sqlite3_bind_int64(statement_, fld, static_cast<sqlite3_int64>(val));}
            template<typename T> inline typename std::enable_if<is_convertible_to_text<T>::value>::type bind_value(T const& val, int fld) {need_step_=true;sqlite3_bind_text(statement_, fld, val.c_str(), (int)val.size(), SQLITE_TRANSIENT);}
        };
        
#pragma mark - sql_types
        
        template<typename T> struct sql_types
        {
            template<typename TFn=T> static inline typename std::enable_if<is_chrono<TFn>::value,std::string>::type const& type_name() {static const std::string ret("REAL"); return ret;}
            template<typename TFn=T> static inline typename std::enable_if<is_convertible_to_float<TFn>::value,std::string>::type const& type_name() {static const std::string ret("REAL"); return ret;}
            template<typename TFn=T> static inline typename std::enable_if<is_convertible_to_int<TFn>::value,std::string>::type const& type_name() {static const std::string ret("INTEGER"); return ret;}
            template<typename TFn=T> static inline typename std::enable_if<is_convertible_to_text<TFn>::value,std::string>::type const& type_name() {static const std::string ret("TEXT"); return ret;}
            
            template<typename TFn=T> static inline typename std::enable_if<is_trivial_container<TFn>::value, std::string>::type table_name() {return to_string() << "_" << type_name<typename TFn::value_type>() << "_table";}
            template<typename TFn=T> static inline typename std::enable_if<is_trivial_map<TFn>::value, std::string>::type table_name() {return to_string() << "_" << type_name<typename TFn::key_type>() << "_to_" << type_name<typename TFn::mapped_type>() << "_table";}
        };
        
#pragma mark - methods
        
        static std::string main_db_name(std::string const&);
        
        sql_inserter_kv create_table(sql_file const& db,std::string const& type);
        sql_inserter_kv create_table_for_array(sql_file const& db,std::string const&, std::string const& typeval);
        sql_inserter_kv create_table_for_map(sql_file const& db,std::string const&, std::string const& typekey, std::string const& typeval);
        std::string create_section(sql_file const& db,std::string const& name, std::string const& path, fn_change_file_name fnch);

        static void create_statements(class_links_container&);
        static std::string sql_order_by(std::string const&) {return "ORDER BY";}
        static std::string sql_order_asc(std::string const& fld) {return to_string() << fld << " ASC";}
        static std::string sql_order_desc(std::string const& fld) {return to_string() << fld << " DESC";}

        static std::string sql_limit(size_t lim) {return to_string() << "LIMIT " << lim;}
        static std::string sql_limit_offset(size_t ofs) {return to_string() << "OFFSET " << ofs;}

        static std::string sql_where() {return "WHERE ";}
        static std::string sql_where(std::string const& fld, std::string const& cond) {return to_string() << fld << cond;}

        static std::string sql_operator_or() {return "OR";}
        static std::string sql_operator_and() {return "AND";}

    private:
        static void create_statements(class_link&, std::string const& relfrom = "");
        size_t create_table_for_versions(sql_file const& db,std::string const& name); // return the version for the 'name'

        string_set created_tables_;
    };
};

#endif /* sb_sqlite_adapter_h */
