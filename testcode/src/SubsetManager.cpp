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

#include "SubsetManager.h"
#include <string>
#include <algorithm>
#include <regex>
#include <ctime>

namespace tars
{

    string SubsetManager::KEY_TYPE = "key";
    string SubsetManager::RATIO_TYPE = "ratio";

    SubsetManager::SubsetManager()
        : _updateTimeInterval(2000), _queryFPrx(nullptr)
    {
    }

    SubsetManager::~SubsetManager()
    {
    }

    int SubsetManager::setQueryPrx(QueryFPrx prx)
    {
        _queryFPrx = prx;
        return 0;
    }

    int SubsetManager::subsetEndpointFilter(string serantName, string routeKey, set<EndpointInfo> &activeEp,
                                            set<EndpointInfo> &inactiveEp, set<EndpointInfo> &subsetActiveEp, set<EndpointInfo> &subsetInactiveEp)
    {
        SubsetConfig config;
        getSubsetConfig(serantName, config);
        subsetInactiveEp=inactiveEp;
        //未启用subset分组
        if (!config.enable)
        {
            subsetActiveEp=activeEp;
            return 0;
        }
        string subsetName = getSubset(serantName, routeKey);
        if (subsetName == "")
        {
            subsetActiveEp=activeEp;
            return 0;
        }
        for (set<EndpointInfo>::iterator iter = activeEp.begin(); iter != activeEp.end(); iter++)
        {
            if (iter->subSet() == subsetName)
            {
                subsetActiveEp.insert(*iter);
            }
            else
            {
                subsetInactiveEp.insert(*iter);
            }
        }
        return 0;
    }

    int SubsetManager::getSubsetConfig(string servantName, SubsetConfig &subsetConf)
    {
        if (cache.find(servantName) != cache.end())
        {
            subsetConf = cache[servantName];
            int64_t tnow = TNOWMS;
            //如果上次更新的时间和当前时间小于间隔，则不更新
            if (subsetConf.lastUpdate + _updateTimeInterval > tnow)
            {
                return 0;
            }
        }

        //从registry获取规则,SubsetConf是由tars生成的结构
        SubsetConf temp_conf;

        _queryFPrx->findSubsetConfigById(servantName, temp_conf);
        subsetConf.ruleType = temp_conf.ruleType;
        //subsetConf.lastUpdate=0;
        if (!temp_conf.enable)
        {
            subsetConf.enable = false;
            return 0;
        }
        subsetConf.enable = true;
        if (temp_conf.ruleType == RATIO_TYPE)
        {
            subsetConf.ruleType = RATIO_TYPE;
            //解析json
            int ret = readRatioRule(temp_conf.ruleData, subsetConf);
        }
        else if (temp_conf.ruleType == KEY_TYPE)
        {
            subsetConf.ruleType = KEY_TYPE;
            //解析json
            int ret = readKeyRule(temp_conf.ruleData, subsetConf);
        }
        else
        {
            //默认的subset的方式
            return 0;
        }
        subsetConf.lastUpdate = TNOWMS;
        cache[servantName] = subsetConf;
        return 0;
    }

    string SubsetManager::getSubset(string servantName, string routeKey)
    {
        SubsetConfig subsetConf;
        int ret = getSubsetConfig(servantName, subsetConf);
        if (ret != 0 || !subsetConf.enable)
        {
            return "";
        }
        if (subsetConf.ruleType == RATIO_TYPE)
        {
            return findSubsetRatio(subsetConf, routeKey);
        }
        else if (subsetConf.ruleType == KEY_TYPE)
        {
            return findSubsetKey(subsetConf, routeKey);
        }
        //默认的路由方式，就返回空的subset字符串
        return "";
    }

    string SubsetManager::findSubsetRatio(SubsetConfig &subsetConf, string routeKey)
    {
        if (subsetConf.ratioConf.size() > 0)
        {
            unsigned int index = 0;
            subsetConf.ratioConf.getIndex(routeKey, index);

            if (index > subsetConf.subsetVectors.size())
            {
                index = index % subsetConf.subsetVectors.size();
            }
            return subsetConf.subsetVectors[index];
        }
        //默认的路由方式，就返回空的subset字符串
        return "";
    }

    string SubsetManager::findSubsetKey(SubsetConfig &subsetConf, string routeKey)
    {
        for (size_t i = 0; i < subsetConf.keyConf.rules.size(); i++)
        {
            KeyRoute rule = subsetConf.keyConf.rules[i];
            if (rule.action == "equal" && routeKey == rule.value)
            {
                return rule.route;
            }
            else if (rule.action == "match")
            {
                std::regex reg(rule.value);
                if (regex_match(routeKey, reg))
                {
                    return rule.route;
                }
            }
        }
        return subsetConf.keyConf.defaultRoute;
    }

    int SubsetManager::readRatioRule(string &ruleData, SubsetConfig &conf)
    {
        JsonValuePtr p = TC_Json::getValue(ruleData);
        if (p.get() == nullptr || p->getType() != eJsonTypeObj)
        {
            char s[128];
            snprintf(s, sizeof(s), "read 'struct' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
            throw tars::TC_Json_Exception(s);
        }
        JsonValueObjPtr Pobj = JsonValueObjPtr::dynamicCast(p);
        int idx = 0;
        conf.subsetVectors.clear();
        conf.ratioConf.clear();
        for (auto iter = Pobj->value.begin(); iter != Pobj->value.end(); iter++)
        {
            string node_name = iter->first;
            int node_weight = 0;
            JsonValuePtr w_ptr = JsonValuePtr::dynamicCast(iter->second);
            readJson(node_weight, w_ptr);
            conf.ratioConf.addNode(node_name, idx, node_weight);
            conf.subsetVectors.push_back(node_name);
            idx++;
        }
        conf.ratioConf.sortNode();
        return 0;
    }

    int SubsetManager::readKeyRule(string &ruleData, SubsetConfig &conf)
    {
        JsonValuePtr p = TC_Json::getValue(ruleData);
        if (p.get() == nullptr || p->getType() != eJsonTypeArray)
        {
            char s[128];
            snprintf(s, sizeof(s), "read 'struct' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
            throw tars::TC_Json_Exception(s);
        }
        JsonValueArrayPtr Parray = JsonValueArrayPtr::dynamicCast(p);
        conf.keyConf.rules.resize(Parray->value.size() - 1);
        int rule_idx = 0;
        for (size_t i = 0; i < Parray->value.size(); i++)
        {
            JsonValueObjPtr Pobj = JsonValueObjPtr::dynamicCast(Parray->value[i]);
            if (Pobj->value.find("equal") != Pobj->value.end())
            {
                conf.keyConf.rules[rule_idx].action = "equal";
                JsonValuePtr value_ptr = JsonValuePtr::dynamicCast(Pobj->value["equal"]);
                JsonValuePtr route_ptr = JsonValuePtr::dynamicCast(Pobj->value["route"]);
                readJson(conf.keyConf.rules[rule_idx].value, value_ptr, true);
                readJson(conf.keyConf.rules[rule_idx].route, route_ptr, true);
                rule_idx++;
            }
            else if (Pobj->value.find("match") != Pobj->value.end())
            {
                conf.keyConf.rules[rule_idx].action = "match";
                JsonValuePtr value_ptr = JsonValuePtr::dynamicCast(Pobj->value["match"]);
                JsonValuePtr route_ptr = JsonValuePtr::dynamicCast(Pobj->value["route"]);
                readJson(conf.keyConf.rules[rule_idx].value, value_ptr, true);
                readJson(conf.keyConf.rules[rule_idx].route, route_ptr, true);
                rule_idx++;
            }
            else if (Pobj->value.find("default") != Pobj->value.end())
            {
                readJson(conf.keyConf.defaultRoute, Pobj->value["default"], true);
            }
            else
            {
                char s[128];
                snprintf(s, sizeof(s), "read 'struct' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
                throw tars::TC_Json_Exception(s);
            }
        }
        return 0;
    }

    void SubsetManager::readJson(int &n, const JsonValuePtr &p, bool is_require)
    {
        if (NULL != p.get() && p->getType() == eJsonTypeNum)
        {
            n = (int)JsonValueNumPtr::dynamicCast(p)->value;
        }
        else if (is_require)
        {
            char s[128];
            snprintf(s, sizeof(s), "read 'Uint16' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
            throw TC_Json_Exception(s);
        }
    }

    void SubsetManager::readJson(string &s, const JsonValuePtr &p, bool is_require)
    {
        if (NULL != p.get() && p->getType() == eJsonTypeString)
        {
            s = JsonValueStringPtr::dynamicCast(p)->value;
        }
        else if (is_require)
        {
            char s[128];
            snprintf(s, sizeof(s), "read 'string' type mismatch, get type: %d.", (p.get() ? p->getType() : 0));
            throw TC_Json_Exception(s);
        }
    }

    void SubsetManager::setSubsetConfig(string servantName, SubsetConfig conf)
    {
        cache[servantName] = conf;
    }

    void SubsetManager::setQuery(bool enable, string ruletype, string ruledata)
    {
        _queryFPrx->_enable = enable;
        _queryFPrx->_ruletype = ruletype;
        _queryFPrx->_ruledata = ruledata;
    }

}