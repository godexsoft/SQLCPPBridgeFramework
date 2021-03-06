//
//  sb_data_section.h
//  SQLCPPBridgeFramework
//
//  Created by Roman Makhnenko on 14/03/2016.
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
#ifndef sb_data_section_h
#define sb_data_section_h

#include "sb_data_section_descriptor.h"

namespace sql_bridge
{
    class data_section;
    typedef std::shared_ptr<data_section> data_sections_ptr;
    typedef std::weak_ptr<data_section> data_sections_weak_ptr;
    typedef std::map<std::string, data_sections_weak_ptr> data_sections_map;
    
    class data_section
    {
    public:
        template<typename T> inline void save(T const& src) {_save<T>(src);}
        template<typename T> inline void load(T& dst, std::string const& flt) {_load<T>(dst,flt);};
        template<typename T> inline void remove(T const& src) {_remove<T>(src);}
        template<typename T> inline void remove_if(std::string const& src) {_remove_if<T>(src);}
        template<typename T> inline typename std::enable_if<is_any_map<T>::value>::type remove_by_key(typename T::key_type const& src) {_remove_by_key<T>(src);}
        template<typename T> inline void replace(T const& src) {_replace<T>(src);}
        template<typename T, typename TMem> inline std::string field_name(TMem const T::*mem_offs) const
        {
            size_t tid = typeid(T).hash_code();
            class_descriptors_ptr desc((*descriptor_)[tid]);
            T const* base((T const*)sizeof(T const*)); // trick for the members detection
            void const* mem_ptr = &(base->*mem_offs);
            for(auto const& mem : desc->members())
                if (mem->is_this_mem_ptr(base, mem_ptr))
                    return mem->field_name();
            return "";
        }
        // statemets creation helpers
        virtual std::string order_by(std::string const&) = 0;
        virtual std::string order_asc(std::string const&) = 0;
        virtual std::string order_desc(std::string const&) = 0;
        virtual std::string limit(size_t) = 0;
        virtual std::string limit_offset(size_t) = 0;
        virtual std::string where() = 0;
        virtual std::string where(std::string const&,std::string const&) = 0;
        virtual std::string operator_or() = 0;
        virtual std::string operator_and() = 0;
        virtual std::string where_between(std::string const&,std::string const&,std::string const&) = 0;
        virtual std::string where_not_between(std::string const&,std::string const&,std::string const&) = 0;
        virtual std::string where_in(std::string const&,std::string const&) = 0;
        virtual std::string where_not_in(std::string const&,std::string const&) = 0;

        virtual ~data_section() {};
    protected:
        data_section(std::string const& sn)
            : descriptor_(data_section_descriptors::instance()[sn])
            {};
        
        virtual data_update_context_ptr create_context(size_t,std::string const& = std::string()) const = 0;
        virtual data_update_context_ptr create_reader(size_t,std::string const&) const = 0;

        data_section_descriptors_ptr descriptor_;
    private:
        
#pragma mark - save
        
        template<typename T> inline typename std::enable_if<!is_container<T>::value && !is_any_map<T>::value>::type _save(T const& src)
        {
            size_t tid = typeid(T).hash_code();
            data_update_context_ptr cont(create_context(tid));
            cont->bind_comp(&src, sql_value());
        }
        template<typename T> inline typename std::enable_if<is_trivial_container<T>::value ||
                                                            is_trivial_map<T>::value ||
                                                            is_container_of_containers<T>::value>::type _save(T const& src)
        {
            size_t tid = typeid(T).hash_code();
            data_update_context_ptr cont(create_context(tid));
            cont->bind_comp(&src,sql_value());
        }
        template<typename T> inline typename std::enable_if<is_container<T>::value &&
                                                            !is_trivial_container<T>::value &&
                                                            !is_container_of_containers<T>::value>::type _save(T const& src)
        {
            if (descriptor_->has_description<T>())
            {
                size_t tid(typeid(T).hash_code());
                data_update_context_ptr cont(create_context(tid));
                cont->bind_comp(&src,sql_value());
            }
            else
            {
                typedef typename types_selector<T>::type type;
                size_t tid = typeid(type).hash_code();
                data_update_context_ptr cont(create_context(tid));
                for(auto const& el : src)
                    cont->bind_comp(&el,sql_value());
            }
        }
        template<typename T> inline typename std::enable_if<is_any_map<T>::value &&
                                                            !is_trivial_map<T>::value &&
                                                            !is_container_of_containers<T>::value>::type _save(T const& src)
        {
            if (descriptor_->has_description<T>())
            {
                size_t tid(typeid(T).hash_code());
                data_update_context_ptr cont(create_context(tid));
                if (is_map<T>::value)
                    for(auto const& el : src)
                        cont->remove_by_key(sql_value(el.first));
                cont->bind_comp(&src,sql_value());
            }
            else
            {
                typedef typename types_selector<T>::type type;
                size_t tid = typeid(type).hash_code();
                data_update_context_ptr cont(create_context(tid));
                for(auto const& el : src)
                    cont->bind_comp(&el.second,sql_value());
            }
        }
        
#pragma mark - replace
        
        template<typename T> inline typename std::enable_if<!is_container<T>::value && !is_any_map<T>::value>::type _replace(T const& src)
        {
            size_t tid = typeid(T).hash_code();
            data_update_context_ptr cont(create_context(tid));
            cont->remove_all();
            cont->bind_comp(&src, sql_value());
        }
        template<typename T> inline typename std::enable_if<is_trivial_container<T>::value ||
                                                            is_trivial_map<T>::value ||
                                                            is_container_of_containers<T>::value>::type _replace(T const& src)
        {
            size_t tid = typeid(T).hash_code();
            data_update_context_ptr cont(create_context(tid));
            cont->remove_all();
            cont->bind_comp(&src,sql_value());
        }
        template<typename T> inline typename std::enable_if<is_container<T>::value &&
                                                            !is_trivial_container<T>::value &&
                                                            !is_container_of_containers<T>::value>::type _replace(T const& src)
        {
            if (descriptor_->has_description<T>())
            {
                size_t tid(typeid(T).hash_code());
                data_update_context_ptr cont(create_context(tid));
                cont->remove_all();
                cont->bind_comp(&src,sql_value());
            }
            else
            {
                typedef typename types_selector<T>::type type;
                size_t tid = typeid(type).hash_code();
                data_update_context_ptr cont(create_context(tid));
                cont->remove_all();
                for(auto const& el : src)
                    cont->bind_comp(&el,sql_value());
            }
        }
        template<typename T> inline typename std::enable_if<is_any_map<T>::value &&
                                                            !is_trivial_map<T>::value &&
                                                            !is_container_of_containers<T>::value>::type _replace(T const& src)
        {
            if (descriptor_->has_description<T>())
            {
                size_t tid(typeid(T).hash_code());
                data_update_context_ptr cont(create_context(tid));
                cont->remove_all();
                cont->bind_comp(&src,sql_value());
            }
            else
            {
                typedef typename types_selector<T>::type type;
                size_t tid = typeid(type).hash_code();
                data_update_context_ptr cont(create_context(tid));
                cont->remove_all();
                for(auto const& el : src)
                    cont->bind_comp(&el.second,sql_value());
            }
        }
        
#pragma mark - load
        
        template<typename T> inline typename std::enable_if<!is_container<T>::value && !is_any_map<T>::value>::type _load(T& dst, std::string const& flt)
        {
            size_t tid = typeid(T).hash_code();
            data_update_context_ptr cont(create_reader(tid, flt));
            cont->read(&dst);
        }
        template<typename T> inline typename std::enable_if<is_trivial_container<T>::value ||
                                                            is_trivial_map<T>::value ||
                                                            is_container_of_containers<T>::value>::type _load(T& dst, std::string const& flt)
        {
            size_t tid = typeid(T).hash_code();
            data_update_context_ptr cont(create_reader(tid, flt));
            cont->read(&dst);
        }
        template<typename T> inline typename std::enable_if<is_container<T>::value &&
                                                            !is_trivial_container<T>::value &&
                                                            !is_container_of_containers<T>::value>::type _load(T& dst, std::string const& flt)
        {
            if (descriptor_->has_description<T>())
            {
                size_t tid = typeid(T).hash_code();
                data_update_context_ptr cont(create_reader(tid, flt));
                cont->read(&dst);
            }
            else
            {
                typedef typename T::value_type type;
                size_t tid = typeid(type).hash_code();
                data_update_context_ptr cont(create_reader(tid, flt));
                type val;
                dst.clear();
                while(cont->is_ok())
                {
                    cont->read(&val);
                    add_to_container(dst, std::move(val));
                }
            }
        }
        template<typename T> inline typename std::enable_if<is_any_map<T>::value &&
                                                            !is_trivial_map<T>::value &&
                                                            !is_container_of_containers<T>::value>::type _load(T& dst, std::string const& flt)
        {
            if (descriptor_->has_description<T>())
            {
                size_t tid = typeid(T).hash_code();
                data_update_context_ptr cont(create_reader(tid, flt));
                cont->read(&dst);
            }
            else
            {
                typedef typename T::key_type k_type;
                typedef typename T::mapped_type m_type;
                size_t tid = typeid(m_type).hash_code();
                data_update_context_ptr cont(create_reader(tid, flt));
                m_type val;
                dst.clear();
                while(cont->is_ok())
                {
                    cont->read(&val);
                    sql_value key = cont->id_for_members(&val);
                    if (key.empty())
                        throw sql_bridge_error(to_string() << "Section: " << descriptor_->section_name() << ". The undefined field for the key", "You should configure any type of index at least at one field in the definition of table");
                    dst.insert(typename T::value_type(key.value<k_type>(),val));
                }
            }
        }
        
#pragma mark - add to container

        template<typename TFn, typename TVal> inline typename std::enable_if<is_back_pushable_container<TFn>::value>::type add_to_container(TFn& dst,TVal&& v) const {dst.push_back(std::move(v));}
        template<typename TFn, typename TVal> inline typename std::enable_if<!is_back_pushable_container<TFn>::value>::type add_to_container(TFn& dst,TVal&& v) const {dst.insert(dst.end(),std::move(v));}

#pragma mark - remove
        
        template<typename T> inline typename std::enable_if<!is_container<T>::value &&
                                                            !is_any_map<T>::value>::type _remove(T const& src)
        {
            size_t tid = typeid(T).hash_code();
            data_update_context_ptr cont(create_context(tid));
            cont->remove_if_possible(&src);
        }
        
        template<typename T> inline typename std::enable_if<is_container<T>::value &&
                                                            !is_trivial_container<T>::value &&
                                                            !is_container_of_containers<T>::value>::type _remove(T const& src)
        {
            size_t tid = typeid(typename T::value_type).hash_code();
            data_update_context_ptr cont(create_context(tid));
            for(auto const& el : src)
                cont->remove_if_possible(&el);
        }

#pragma mark - remove_by_key
        
        template<typename T> inline typename std::enable_if<is_any_map<T>::value>::type _remove_by_key(typename T::key_type const& val)
        {
            size_t tid = typeid(T).hash_code();
            data_update_context_ptr cont(create_context(tid));
            cont->remove_by_key(sql_value(val));
        }
        
#pragma mark - remove_if
        
        template<typename T> inline typename std::enable_if<!is_container<T>::value &&
                                                            !is_any_map<T>::value>::type _remove_if(std::string const& flt)
        {
            size_t tid = typeid(T).hash_code();
            data_update_context_ptr cont(create_context(tid,flt));
            cont->remove_all();
        }
        
    };

    template<typename TStrategy> class _t_data_read_context
        : public data_update_context
    {
    public:
        _t_data_read_context(typename TStrategy::sql_file const& fl,
                             class_descriptors_ptr desc,
                             class_link const& lnk,
                             data_section_descriptors_ptr hr,
                             std::string const& flt,
                             sql_value const& extkey)
            : data_update_context(desc,lnk)
            , file_(fl)
            , hierarhy_(hr)
            , reader_(fl,to_string() << lnk.statements().select_ << " " << (flt.empty()?lnk.statements().select_app_:flt), extkey)
            , last_key_(0)
            , use_ext_primary_key_(lnk.index_ref().type()==e_db_key_mode::ExternalPrimaryKey)
        {
            if (!use_ext_primary_key_)
            {
                std::string ifn(lnk.index_ref().name());
                for(auto const& m : members())
                    if (m->field_name()==ifn)
                {
                    member_for_id_ = m;
                    break;
                }
            }
        }
        
        bool next(void const*) override
        {
            if (use_ext_primary_key_)
                reader_.read_value(last_key_);
            return reader_.next();
        }

        void add(sql_value const& var) override {}
        void remove_if_possible(void const*) override {}
        void remove_all() override {}
        void remove_by_key(sql_value const&) override {}
        bool is_ok() override {return reader_.is_valid();}
        void check_for_update_ability(void const*) override {}
        
        void read(sql_value& dst) override
        {
            if (reader_.is_null())
                dst = sql_value();
            else
            switch (dst.type_)
            {
                case sql_value::e_key_type::Integer:
                    reader_.read_value(dst.iValue_);
                    break;
                case sql_value::e_key_type::Real:
                    reader_.read_value(dst.rValue_);
                    break;
                case sql_value::e_key_type::String:
                    reader_.read_value(dst.tValue_);
                    break;
                default:
                    throw sql_bridge_error(to_string() << "Table: " << table_name() <<". " << g_internal_error_text, g_architecture_error_text);
            }
        }
        
        sql_value id_for_members(void const* dat) const override
        {
            if (use_ext_primary_key_)
                return sql_value(last_key_);
            return member_for_id_?member_for_id_->expand(dat):sql_value();
        }

        data_update_context_ptr context_for_member(size_t etid, sql_value const& extkey, std::string const& ref) override
        {
            for(auto const& tl : link_.target())
                if (tl.source_id()==etid && ref==tl.ref_field_name())
                    return data_update_context_ptr(new _t_data_read_context<TStrategy>(file_,(*hierarhy_)[etid],tl,hierarhy_,"",extkey));
            throw sql_bridge_error(to_string() << "Table: " << table_name() <<". " << g_internal_error_text, g_architecture_error_text);
        }

    private:
        typename TStrategy::sql_file const& file_;
        data_section_descriptors_ptr hierarhy_;
        class_descriptors_ptr member_for_id_;
        typename TStrategy::sql_reader reader_;
        int64_t last_key_;
        bool use_ext_primary_key_;
    };

    template<typename TStrategy, typename TTransactionLock> class _t_data_update_context
        : public data_update_context
        , private TTransactionLock
    {
    public:
        _t_data_update_context(typename TStrategy::sql_file const& fl, class_descriptors_ptr desc, class_link const& lnk, data_section_descriptors_ptr hr, std::string const& flt)
            : data_update_context(desc,lnk)
            , TTransactionLock(fl)
            , file_(fl)
            , inserter_(fl,link_.statements().insert_)
            , remover_(fl,link_.statements().remove_)
            , remover_for_all_(fl,link_.statements().remove_all_.empty()?std::string():(to_string() << link_.statements().remove_all_ << " " << flt))
            , updater_(fl,link_.statements().update_)
            , hierarhy_(hr)
            , last_insert_id_(0)
            , use_last_id_(false)
            , update_mode_(false)
        {
            if (link_.index_ref().type()==e_db_key_mode::ExternalPrimaryKey ||
                link_.index_ref().type()==e_db_key_mode::PrimaryKey)
                    use_last_id_ = true;
            
            std::string ifn(link_.index_ref().name());
            if (!ifn.empty())
                for(auto const& m : members())
                    if (m->field_name()==ifn)
            {
                member_for_id_ = m;
                break;
            }
        }
        
        bool is_ok() override {return true;}
        void add(sql_value const& var) override
        {
            if (update_mode_)
                updater_.bind(var);
            else
                inserter_.bind(var);
        }
        void read(sql_value&) override {}
        data_update_context_ptr context_for_member(size_t etid, sql_value const&, std::string const& ref) override
        {
            for(auto const& tl : link_.target())
                if (tl.source_id()==etid && ref==tl.ref_field_name())
                    return data_update_context_ptr(new _t_data_update_context<TStrategy,typename TStrategy::sql_file::no_transactions_lock>(file_,(*hierarhy_)[etid],tl,hierarhy_,""));
            throw sql_bridge_error(to_string() << "Table: " << table_name() <<". " << g_internal_error_text, g_architecture_error_text);
        }
        bool next(void const* dat) override
        {
            if (update_mode_)
            {
                sql_value key = member_for_id_->expand(dat);
                updater_.bind(key);
                updater_.mark_as_dirty();
                return updater_.next();
            }
            else
            {
                inserter_.mark_as_dirty();
                bool ret = inserter_.next();
                if (use_last_id_)
                    last_insert_id_ = file_.last_insert_id();
                return ret;
            }
        }
        
        sql_value id_for_members(void const* dat) const override
        {
            if (update_mode_)
                return member_for_id_->expand(dat);
            if (member_for_id_ && !use_last_id_)
                return member_for_id_->expand(dat);
            return use_last_id_?sql_value(last_insert_id_):sql_value();
        }
        
        void check_for_update_ability(void const* dat) override
        {
            update_mode_ = false;
            if (updater_.empty() || !use_last_id_ || !member_for_id_ || remove_all_used_) return;
            sql_value val = member_for_id_->expand(dat);
            if (val.empty()) return;
            update_mode_ = val.value<int64_t>()!=0;
        }
        
        void remove_if_possible(void const* dat) override
        {
            if (update_mode_) return;
            if (remover_.empty() || !member_for_id_ || remove_all_used_) return;
            sql_value mid = member_for_id_->expand(dat);
            remover_.bind(mid);
            remover_.next();
        }

        void remove_by_key(sql_value const& mid) override
        {
            if (remover_.empty()|| remove_all_used_) return;
            remover_.bind(mid);
            remover_.next();
        }

        void remove_all() override
        {
            remove_all_used_ = true;
            if (remover_for_all_.empty()) return;
            remover_for_all_.mark_as_dirty();
            remover_for_all_.next();
        }
        
    private:
        typename TStrategy::sql_file const& file_;
        typename TStrategy::sql_updater inserter_;
        typename TStrategy::sql_updater remover_;
        typename TStrategy::sql_updater remover_for_all_;
        typename TStrategy::sql_updater updater_;
        data_section_descriptors_ptr hierarhy_;
        int64_t last_insert_id_;
        class_descriptors_ptr member_for_id_;
        bool use_last_id_,update_mode_;
    };

    template<typename TStrategy> class _t_data_section
        : public data_section
        , private TStrategy::sql_file
    {
    public:
        _t_data_section(std::string const& fname,std::string const& sectname)
            : data_section(sectname)
            , TStrategy::sql_file(fname)
            {};
    private:
        data_update_context_ptr create_context(size_t tid, std::string const& flt) const override
        {
            class_descriptors_ptr desc = (*descriptor_)[tid];
            return data_update_context_ptr(new _t_data_update_context<TStrategy,typename TStrategy::sql_file::transactions_lock>(*this,desc,desc->depends(),descriptor_,flt));
        }
        data_update_context_ptr create_reader(size_t tid, std::string const& flt) const override
        {
            class_descriptors_ptr desc = (*descriptor_)[tid];
            return data_update_context_ptr(new _t_data_read_context<TStrategy>(*this,desc,desc->depends(),descriptor_,flt,sql_value()));
        }
        
        std::string order_by(std::string const& fld) override {return TStrategy::sql_order_by(fld);}
        std::string order_asc(std::string const& fld) override {return TStrategy::sql_order_asc(fld);}
        std::string order_desc(std::string const& fld) override {return TStrategy::sql_order_desc(fld);}
        std::string limit(size_t lim) override {return TStrategy::sql_limit(lim);}
        std::string limit_offset(size_t ofs) override {return TStrategy::sql_limit_offset(ofs);}
        std::string where() override {return TStrategy::sql_where();}
        std::string where(std::string const& fld,std::string const& cnd) override {return TStrategy::sql_where(fld,cnd);}
        std::string operator_or() override {return TStrategy::sql_operator_or();}
        std::string operator_and() override {return TStrategy::sql_operator_and();}
        std::string where_between(std::string const& fld,std::string const& from,std::string const& to) override {return TStrategy::sql_where_between(fld,from,to);}
        std::string where_not_between(std::string const& fld,std::string const& from,std::string const& to) override {return TStrategy::sql_where_not_between(fld,from,to);}
        std::string where_in(std::string const& fld,std::string const& val) override {return TStrategy::sql_where_in(fld,val);}
        std::string where_not_in(std::string const& fld,std::string const& val) override {return TStrategy::sql_where_not_in(fld,val);}

    };
    
};

#endif /* sb_data_section_h */
