/*
 * @Author: your name
 * @Date: 2021-08-25 10:55:33
 * @LastEditTime: 2021-09-07 11:07:04
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /CppVScodeTemplate/main.cpp
 */
#include "SubsetManager.h"
#include <ctime>

using namespace tars;

string strRand(int length) {			// length: 产生字符串的长度
    char tmp;							// tmp: 暂存一个随机数
    string buffer;						// buffer: 保存返回值
    
    // 下面这两行比较重要:
    random_device rd;					// 产生一个 std::random_device 对象 rd
    default_random_engine random(rd());	// 用 rd 初始化一个随机数发生器 random
    
    for (int i = 0; i < length; i++) {
        tmp = random() % 36;	// 随机一个小于 36 的整数，0-9、A-Z 共 36 种字符
        if (tmp < 10) {			// 如果随机数小于 10，变换成一个阿拉伯数字的 ASCII
            tmp += '0';
        } else {				// 否则，变换成一个大写字母的 ASCII
            tmp -= 10;
            tmp += 'A';
        }
        buffer += tmp;
    }
    return buffer;
}

EndpointF make_endpointf(string host,int port,int timeout,int istcp,int grid,
                        int groupworkid,int grouprealid,string setid,int qos,
                        int bakflag,int weight,int weighttype,int authtype,string subset)
{
    EndpointF temp;
    temp.host=host;
    temp.port=port;
    temp.timeout=timeout;
    temp.istcp=istcp;
    temp.grid=grid;
    temp.groupworkid=groupworkid;
    temp.grouprealid=grouprealid;
    temp.setId=setid;
    temp.qos=qos;
    temp.bakFlag=bakflag;
    temp.weight=weight;
    temp.weightType=weighttype,
    temp.authType=authtype;
    temp.subset=subset;
    return temp;
}

void print_endpointinfo(const EndpointInfo& ep)
{
    printf("EndpointInfo( host:%s, port:%d, authType:%d, subset:%s )\n",ep.host().c_str(),ep.port(),ep.authType(),ep.subSet().c_str());
}

void add_endpoint(set<EndpointInfo>& eps)
{
    eps.insert(EndpointInfo(make_endpointf("host1",1,2,3,4,5,6,"set1",7,8,9,10,11,"v1")));
    eps.insert(EndpointInfo(make_endpointf("host2",1,2,3,4,5,6,"set1",7,8,9,10,11,"v1")));
    eps.insert(EndpointInfo(make_endpointf("host3",1,2,3,4,5,6,"set1",7,8,9,10,11,"v2")));
    eps.insert(EndpointInfo(make_endpointf("host4",1,2,3,4,5,6,"set1",7,8,9,10,11,"v2")));
    eps.insert(EndpointInfo(make_endpointf("host5",1,2,3,4,5,6,"set1",7,8,9,10,11,"v2")));
    eps.insert(EndpointInfo(make_endpointf("host6",1,2,3,4,5,6,"set1",7,8,9,10,11,"v3")));
}

/**
 * @description: 按比例路由的多次测试 
 * @param {*}
 * @return {*}
 */
void ratio_test2()
{
    cout<<"Start Ratio Route Test 2:\n";
    set<EndpointInfo> eps;
    set<EndpointInfo> inactive;
    add_endpoint(eps);
    srand(time(0));
    SubsetManager subset_manager;
    //设置subset的规则
    QueryFPrx temp=new QueryPrx();
    subset_manager.setQueryPrx(temp);
    subset_manager.setQuery(true,"ratio",R"({"v1": 20, "v2": 80, "v3": 60})");

    map<string,int> counter;
    int N=1e6;
    cout<<"一共测试了 "<<N<<" 次"<<endl;
    for(int i=0;i<N;i++)
    {
        string rkey=strRand(8);
        set<EndpointInfo> s_active,s_inactive;
        int ret=subset_manager.subsetEndpointFilter("test",rkey,eps,inactive,s_active,s_inactive);
        string subset=s_active.begin()->subSet();
        counter[subset]++;
    }
    for(auto temp:counter){
        cout<<"路由到"<<temp.first<<": "<<temp.second<<endl;
    }
    cout<<endl;
}

/**
 * @description: 按key值规律端点集合
 * @param {*}
 * @return {*}
 */
void key_test2()
{
    cout<<"Start Key Route Test 2:\n";
    set<EndpointInfo> eps;
    set<EndpointInfo> inactive;
    add_endpoint(eps);
    SubsetManager subset_manager;
    //设置subset的规则
    QueryFPrx temp=new QueryPrx();
    subset_manager.setQueryPrx(temp);
    subset_manager.setQuery(true,"key",R"([{"equal":"100", "route":"v1"}, {"match":"22*", "route":"v2"},{"default":"v3"}])");

    //equal的结果
    cout<<"\nEqual route 100 result:\n";
    set<EndpointInfo> active=eps;
    set<EndpointInfo> s_active,s_inactive;
    int ret=subset_manager.subsetEndpointFilter("test","100",active,inactive,s_active,s_inactive);
    for(auto iter=s_active.begin();iter!=s_active.end();iter++){
        print_endpointinfo(*iter);
    }

    //match的结果
    cout<<"\nMatch route 2222 result:\n";
    active=eps;
    s_active.clear();
    s_inactive.clear();
    ret=subset_manager.subsetEndpointFilter("test","2222",active,inactive,s_active,s_inactive);
    for(auto iter=s_active.begin();iter!=s_active.end();iter++){
        print_endpointinfo(*iter);
    }

    //match的结果
    cout<<"\nMatch route 22 result:\n";
    active=eps;
    s_active.clear();
    s_inactive.clear();
    ret=subset_manager.subsetEndpointFilter("test","22",active,inactive,s_active,s_inactive);
    for(auto iter=s_active.begin();iter!=s_active.end();iter++){
        print_endpointinfo(*iter);
    }

    //default的结果
    cout<<"\nDefault route result:\n";
    active=eps;
    s_active.clear();
    s_inactive.clear();
    ret=subset_manager.subsetEndpointFilter("test","300",active,inactive,s_active,s_inactive);
    for(auto iter=s_active.begin();iter!=s_active.end();iter++){
        print_endpointinfo(*iter);
    }
}

void conhash_test()
{
    TC_ConsistentHashNew method(E_TC_CONHASH_KETAMAHASH);
    method.addNode("v1",0,20);
    method.addNode("v2",1,160);
    method.sortNode();

    method.printNode();
    vector<int> counter(2,0);
    int N=1e5;
    for(int i=0;i<N;i++){
        unsigned int idx=0;
        string key=strRand(8);
        method.getIndex(key,idx);
        counter[idx]++;
    }
    for(int i=0;i<2;i++){
        cout<<i<<": "<<counter[i]<<endl;
    }

}

int main()
{
    //ratio_test1();

    //conhash_test();
    //按比例路由测试
    ratio_test2();
    //按键值路由测试
    key_test2();
}