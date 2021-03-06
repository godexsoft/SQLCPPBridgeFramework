//
//  main.cpp
//  SQLCPPBridgeFramework
//
//  Created by Roman Makhnenko on 11/03/2018.
//  Copyright © 2018, DataArt.
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

#include <iostream>
#include <sys/stat.h>
#include "sqlcppbridge.h"
#include "time_tracker.h"

#include "example1.h"
#include "example2.h"
#include "example3.h"
#include "example4.h"
#include "example5.h"
#include "example6.h"
#include "example7.h"
#include "example8.h"
#include "example9.h"
#include "example10.h"
#include "example11.h"
#include "example12.h"
#include "example13.h"
#include "example14.h"
#include "example15.h"
#include "example16.h"
#include "example17.h"
#include "example18.h"
#include "example19.h"
#include "example20.h"
#include "example21.h"
#include "example22.h"
#include "example23.h"
#include "example24.h"
#include "example25.h"
#include "example26.h"
#include "example27.h"
#include "example28.h"
#include "example29.h"

int main(int argc, char** argv)
{
    time_tracker tracker;
    try
    {
        mkdir("./DB", 0777);
        sql_bridge::local_storage<sql_bridge::sqlite_adapter> storage("./DB");
        
#if 1
        {
            std::cout << "Case KVDB ";
            time_tracker trk;
            // test kvdb
            double src_rl(M_PI),dst_rl;
            std::string src_str("aaa"),dst_str;
            int src_int(100500),dst_int;

            storage.save("test", src_rl);
            storage.save("test", src_str);
            storage.save("test", src_int);

            dst_rl = storage.load("test",0.0);
            dst_str = storage.load("test",std::string());
            dst_int = storage.load("test",0);

            assert(src_rl==dst_rl);
            assert(src_str==dst_str);
            assert(src_int==src_int);

            std::vector<std::string> arrstr1_src,arrstr1_dst;
            for(int i=0; i<1000; i++)
                arrstr1_src.push_back(sql_bridge::to_string() << "FF" << i);
            storage.save("FFStrings", arrstr1_src);
            arrstr1_dst = storage.load("FFStrings", std::vector<std::string>());
            assert(arrstr1_src==arrstr1_dst);

            std::set<std::string> setstr1_src,setstr1_dst;
            for(int i=0; i<10000; i++)
                setstr1_src.insert(sql_bridge::to_string() << "FF" << i);
            storage.save("FFSet", setstr1_src);
            setstr1_dst = storage.load("FFSet", std::set<std::string>());
            assert(setstr1_src==setstr1_dst);

            std::array<char,20> stat_src,stat_dst;
            std::fill(stat_src.begin(),stat_src.end(),10);
            storage.save("STAT", stat_src);
            stat_dst = storage.load("STAT", std::array<char,20>());
            assert(stat_src==stat_dst);
            
            std::map<int,std::string> mapstr1_src,mapstr1_dst;
            for(int i=0; i<1000; i++)
                mapstr1_src[i] = sql_bridge::to_string() << "Map: " << i+1;
            storage.save("IntStringMap",mapstr1_src);
            mapstr1_dst = storage.load("IntStringMap",std::map<int,std::string>());
            assert(mapstr1_src==mapstr1_dst);
            
            std::chrono::system_clock::time_point srcnow(std::chrono::system_clock::now()),dstnow;
            storage.save("NOW", srcnow);
            dstnow = storage.load("NOW", std::chrono::system_clock::time_point());
            assert(srcnow==dstnow);
            
            std::cout << "is ok. ";
        }
        
        {
            time_tracker trk;
            sql_bridge::context cont(storage["case1"]);
            std::cout << "Case 1 ";
            std::map<int,Case1> src1,dst1;
            Case1 chk(0);
            for(int i=0; i<1000; ++i)
                src1.insert({i,Case1(i)});
            
            cont.save(src1);
            cont.load(dst1);
            cont.load(chk,"WHERE ID=10");

            assert(src1==dst1);
            assert(src1.find(10)->second==chk);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case2"]);
            std::cout << "Case 2 ";

            std::vector<Case2> src2,dst2;
            src2.reserve(10000);
            for(int i=0; i<10000; ++i)
                src2.push_back(Case2(i*3));

            cont.save(src2);
            cont.load(dst2);
            assert(src2==dst2);

            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case3"]);
            std::cout << "Case 3 ";

            std::deque<Case3> src3,dst3;
            std::map<long,Case3> dstmap;
            for(int i=0; i<100; ++i)
                src3.push_back(Case3(i+1));
            
            cont.save(src3);
            cont.remove(src3[50]);
            src3.erase(src3.begin()+50);
            cont.load(dst3);
            cont.load(dstmap);
            assert(src3==dst3);
            cont.load(std::vector<Case3>(),"WHERE ID>30", [](std::vector<Case3>&& rv)
            {
                std::cout << std::endl << rv.size() << " element(s) have been loaded asynchroniously for the Case 3." << std::endl;
            });
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case4"]);
            std::cout << "Case 4 ";
            std::vector<Case4> src4,dst4;
            for(long long i=0; i<10000; ++i)
                src4.push_back(Case4(i+1));
            cont.replace(src4);
            cont.load(dst4);
            assert(src4==dst4);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case5"]);
            std::cout << "Case 5 ";
            Case5a src5a, dst5a;
            Case5b src5b, dst5b;
            for(long long i=0; i<100; ++i)
                src5a.push_back(sql_bridge::to_string() << "var: " << i+1);
            for(long long i=0; i<300; ++i)
                src5b[sql_bridge::to_string() << "key: " << i+1] = i;

            cont.replace(src5a);
            cont.replace(src5b);
            
            cont.load(dst5a);
            cont.load(dst5b);
            assert(src5a==dst5a);
            assert(src5b==dst5b);

            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case6"]);
            std::cout << "Case 6 ";
            std::vector<Case6> src6,dst6;
            for(long long i=0; i<10; ++i)
                src6.push_back(Case6(i));
            cont.save(src6);
            cont.load(dst6);
            assert(src6==dst6);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case7"]);
            std::cout << "Case 7 ";
            std::vector<Case7> src7,dst7;
            for(long long i=0; i<10; ++i)
                src7.push_back(Case7(i));
            cont.replace(src7);
            std::set<Case7> rmset({src7[3],src7[4]});
            cont.remove(rmset);
            src7.erase(src7.begin()+4);
            src7.erase(src7.begin()+3);
            cont.load(dst7);
            assert(src7==dst7);
            std::cout << "is ok. ";
        }
        
        {
            time_tracker trk;
            sql_bridge::context cont(storage["case8"]);
            std::cout << "Case 8 ";
            std::set<Case8> src8,dst8;
            for(int i=0; i<10; ++i)
                src8.insert(Case8(i));
            cont.save(src8);
            cont.load(dst8);
            assert(src8==dst8);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case9"]);
            std::cout << "Case 9 ";
            Case9 src9,dst9;
            for(int i=0; i<10; ++i)
                src9.insert(Case9::value_type(i,Case9Element(10,Case9Member(i))));
            cont.replace(src9);
            cont.load(dst9);
            assert(src9==dst9);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case10"]);
            std::cout << "Case 10 ";
            Case10 src10,dst10;
            for(int i=0; i<10; ++i)
                src10.push_back(Case10Element(20,i*5));
            cont.replace(src10);
            cont.load(dst10);
            assert(src10==dst10);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage.context_with_alt_file("case11", [](std::string const& src)->std::string
            {
                return sql_bridge::to_string() << "test_" << src;
            }));
            std::cout << "Case 11 ";
            Case11 src11(20),dst11;
            cont.save(src11);
            cont.load(dst11);
            assert(src11==dst11);
            std::cout << "is ok. ";
        }
        
        {
            time_tracker trk;
            sql_bridge::context cont(storage["case12"]);
            std::cout << "Case 12 ";
            Case12 src12(10),dst12;
            cont.save(src12);
            cont.load(dst12);
            assert(src12==dst12);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case13"]);
            std::cout << "Case 13 ";
            cont.save(Case13(EBase::Normal));
            cont.save(Case13(EBase::Normal));
            cont.save(Case13(EBase::Alt));
            std::vector<Case13> src13({Case13(EBase::Normal),Case13(EBase::Alt)}),dst13;
            cont.load(dst13);
            assert(src13==dst13);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case14"]);
            std::cout << "Case 14 ";
            std::vector<Case14> src14,dst14;
            src14.reserve(100);
            for(long i=0; i<100;++i)
                src14.push_back(Case14(i));
            cont.save(src14);
            src14[30].members_[10] = Case14First(20);
            cont.save(src14[30]);
            cont.load(dst14,"ORDER BY ID");
            assert(src14==dst14);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case15"]);
            std::cout << "Case 15 ";

            std::vector<Case15> src15,dst15;
            src15.reserve(100);
            for(long i=0; i<100;++i)
                src15.push_back(Case15(i));
            cont.replace(src15);
            cont.load(src15);
            cont.remove(src15[20]);
            src15.erase(src15.begin()+20);
            cont.load(dst15);
            assert(src15==dst15);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case16"]);
            std::cout << "Case 16 ";

            std::map<int64_t,Case16> src16,dst16;
            for(long i=0; i<100;++i)
                src16.insert({i,Case16(i)});
            cont.save(src16);
            cont.load(dst16);
            assert(src16==dst16);
            std::cout << "is ok. ";
        }
        
        {
            time_tracker trk;
            sql_bridge::context cont(storage["case17"]);
            std::cout << "Case 17 ";
            _TLongToCase17Map src17,dst17;
            for(long i=0; i<100;++i)
                src17.insert({i,Case17(i,true)});
            cont.replace(src17);
            cont.load(dst17);
            assert(src17==dst17);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case18"]);
            std::cout << "Case 18 ";
            Case18 src18(10),dst18;
            cont.save(src18);
            cont.load(dst18);
            assert(src18==dst18);
            std::cout << "is ok. ";
        }
        
        {
            time_tracker trk;
            sql_bridge::context cont(storage["case19"]);
            std::cout << "Case 19 ";
            Case19Map src19
            ({
                Case19(1).value(),
                Case19(1).value(),
                Case19(2).value(),
                Case19(3).value(),
                Case19(4).value(),
            }),dst19;
            cont.replace(src19);
            cont.load(dst19);
            assert(src19==dst19);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case20"]);
            std::cout << "Case 20 ";
            Case20Container src20,dst20;
            for(int i=0; i<100;++i)
                src20.push_back(Case20(i+1));
            cont.replace(src20);
            cont.load(dst20);
            assert(src20==dst20);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case21"]);
            std::cout << "Case 21 ";
            std::vector<Case21> src21,dst21;
            for(int i=0; i<100; ++i)
                src21.push_back(Case21(i+20));
            cont.replace(src21);
            cont.load(dst21,"WHERE V=21");
            assert(src21[1]==dst21[0]);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case22"]);
            std::cout << "Case 22 ";
            Case22Container src =
            {
                {"key_11",Case22(1)},
                {"key_12",Case22(2)},
            },dst;
            cont.save(src);
            cont.where(&Case22::data_,"=",1l)
                .sql_or()
                .where(&Case22::key_,"=",std::string("key_12"))
                .order_desc(&Case22::key_)
                .load(dst);
            assert(src==dst);
            std::cout << "is ok. ";
        }
        
        {
            time_tracker trk;
            sql_bridge::context cont(storage["case23"]);
            std::cout << "Case 23 ";
            Case23Container src,dst;
            for(int i=0; i<100; ++i)
                src.push_back(i);
            cont.save(src);
            cont.where_not_between(&Case23::data_, 10l, 90l)
                .remove_if<Case23>();
            cont.where_in(&Case23::data_,std::set<long>({50,60}))
                .remove_if<Case23>();
            Case23Container chk;
            std::remove_copy_if(src.begin(),
                                src.end(),
                                std::back_inserter(chk),
                                [](Case23 const& v){return v.data_<10 || v.data_>90 || v.data_==50 || v.data_==60;});
            cont.load(dst);
            assert(chk==dst);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case24"]);
            std::cout << "Case 24 ";
            Case24Container src,dst;
            for(int i=0; i<100; ++i)
                src.push_back(i);
            cont.save(src);
            cont.where_between(60l, &Case24::lower_, &Case24::upper_)
                .order(&Case24::lower_)
                .load(dst);
            Case24Container chk;
            std::copy_if(src.begin(),
                         src.end(),
                         std::back_inserter(chk),
                         [](Case24 const& v){return 60>=v.lower_ && 60<=v.upper_ && v.lower_<=v.upper_;});
            assert(chk==dst);
            std::cout << "is ok. ";
        }
        
        {
            time_tracker trk;
            sql_bridge::context cont(storage["case25"]);
            std::cout << "Case 25 ";
            Case25Container src,dst;
            for(int i=0; i<100; ++i)
                src.push_back(i);
            cont.save(src);
            cont.load(dst);
            assert(src==dst);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case26"]);
            std::cout << "Case 26 ";
            Case26Container src,dst;
            for(int i=0; i<100; ++i)
                src.push_back(i);
            cont.save(src);
            cont.load(dst);
            assert(src==dst);
            std::cout << "is ok. ";
        }
        {
            time_tracker trk;
            sql_bridge::context cont(storage["case27"]);
            std::cout << "Case 27 ";
            Case27Container src,dst;
            for(int i=0; i<100; ++i)
                src.insert({i,Case27(i+100)});
            cont.save(src);
            cont.remove<Case27Container>(5);
            cont.remove<Case27Container>(17);
            src.erase(5);
            src.erase(17);
            cont.load(dst);
            assert(src==dst);
            std::cout << "is ok. ";
        }
        {
            time_tracker trk;
            sql_bridge::context cont(storage["case28"]);
            std::cout << "Case 28 ";
            Case28 src;
            std::vector<Case28> chk;
            src.fill(10);
            chk.push_back(src);
            cont.replace(src);
            src.fill(20);
            chk.push_back(src);
            cont.save(src);
            std::vector<Case28> dst,dst2;
            cont.load(dst);
            dst[1].extra_++;
            cont.save(dst[1]);
            cont.load(dst2);
            assert(dst==dst2);
            std::cout << "is ok. ";
        }

        {
            time_tracker trk;
            sql_bridge::context cont(storage["case29"]);
            std::cout << "Case 29 ";
            Case29Container src,dst;
            for(int i=0; i<10; ++i)
                src.push_back(Case29(i));
            cont.save(src);
            cont.load(dst);
            assert(src==dst);
            std::cout << "is ok. ";
        }
#endif

    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Done! ";
    return EXIT_SUCCESS;
}
