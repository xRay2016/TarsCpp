/**
 * Tencent is pleased to support the open source community by making Tars available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 */

#ifndef _TARS_SUBSET_MANAGER_H_
#define _TARS_SUBSET_MANAGER_H_

#include "util/tc_json.h"
#include "util/tc_consistent_hash_new.h"
#include "util/tc_timeprovider.h"
#include "EndpointInfo.h"
//#include "servant/QueryF.h"
#include <string>
#include <map>
#include <vector>
#include <set>


namespace tars
{

/*
* 下面是调试用的信息
*/

/*
struct SubsetConf
{
    bool enable;
    string ruleType;
    string ruleData;
};
*/
class QueryPrx
{
public:
    int findSubsetConfigById(const std::string & id,tars::SubsetConf &conf)
    {
        conf.enable=_enable;    
        conf.ruleType=_ruletype;
        conf.ruleData=_ruledata;
        return 0;
    }
    bool _enable;
    std::string _ruletype;
    std::string _ruledata;
};
typedef QueryPrx* QueryFPrx;


struct KeyRoute
{
    string action;
    string value;
    string route;
};

/* 
* 按关键值匹配的subset路由参数
*/
struct KeyConfig
{
    vector<KeyRoute> rules;
    string defaultRoute;
};

/*
* subset规则参数
*/
struct SubsetConfig
{
    SubsetConfig():ratioConf(E_TC_CONHASH_KETAMAHASH)
    {

    }
    bool enable;                          //是否启用subset
    string ruleType;                      //subset规则
    TC_ConsistentHashNew ratioConf;       //比例规则参数，使用一致性哈希
    vector<string> subsetVectors;         //比例规则参数，存储所有的subset名
    KeyConfig keyConf;                    //路由按值匹配规则参数
    int64_t lastUpdate;                   //上次更新subset规则的时间
};

////////////////////////////////////////////////////////////////////////
/*
 * 框架内部的subset路由管理的实现类
 */
class SubsetManager
{
public:
    static std::string RATIO_TYPE;//比例参数的subset
    static std::string KEY_TYPE;//关键词参数的subset

public:
    /*
     * 构造函数
     */
    SubsetManager();

    /*
     * 析构函数
     */
    ~SubsetManager();

    /**
     * @description: 根据subset规则过滤活跃节点
     * @param {string} serantName servant名字
     * @param {string} routeKey 路由key
     * @param {set<EndpointInfo>} activeEp filter前活跃的节点
     * @param {set<EndpointInfo>} inactiveEp filter前不活跃的节点
     * @param {set<EndpointInfo>} subsetActiveEp filter过后活跃的节点
     * @param {set<EndpointInfo>} subsetInactiveEp filter过后活跃的节点
     * @return {*} 返回处理的结果值 
     */  
    int subsetEndpointFilter(string serantName,string routeKey,set<EndpointInfo>& activeEp,
    set<EndpointInfo>& inactiveEp,set<EndpointInfo>& subsetActiveEp,set<EndpointInfo>& subsetInactiveEp);

    /**
     * @description: 设置QueryF的指针
     * @param {QueryFPrx} prx
     */    
    int setQueryPrx(QueryFPrx prx);

    //test的接口
    void setSubsetConfig(string servantName,SubsetConfig conf);
    void setQuery(bool enable,string ruletype,string ruledata);

private:
    string getSubset(string servantName,string routeKey);

    int getSubsetConfig(string servantName,SubsetConfig& subsetConf);
    
    string findSubsetRatio(SubsetConfig& subsetConf,string routeKey);

    string findSubsetKey(SubsetConfig& subsetConf,string routeKey);

    int readRatioRule(string& ruleData,SubsetConfig& subsetConf);

    int readKeyRule(string& ruleData,SubsetConfig& subsetConf);

    void readJson(int& n,const JsonValuePtr& p,bool is_require=true);

    void readJson(std::string& s, const JsonValuePtr & p, bool isRequire=true);

    map<string,SubsetConfig> cache;

    int64_t _updateTimeInterval;

    /*
    * 一致性哈希算法使用
    */
    TC_ConsistentHashNew _consistenHashWeight;

    QueryFPrx _queryFPrx;
};
}

#endif