#include<bits/stdc++.h>
#include "parameters.h"
using namespace std;



unordered_map<string,int> serverType2Index;  //服务器型号->清单上的编号
unordered_map<string,int> vmType2Index;  //虚拟机型号->清单上的编号

struct ServerInfo{
    string type;
    int core;
    int memory;
    int serverCost;
    int powerCost;

    void read_std(){
        static char model[22];
        scanf("%s%d,%d,%d,%d)",model,&this->core, &this->memory, &this->serverCost,&this->powerCost);
        model[strlen(model)-1]='\0';  //去掉末尾多输入的逗号
        this->type=string(model+1);  //model去掉首个字符左括号

        serverType2Index[this->type]=serverType2Index.size();
    }
};

struct VMInfo{
    string type;
    int core;
    int memory;
    int two;

    void read_std(){
        static char model[22];
        scanf("%s%d,%d,%d)",model,&this->core, &this->memory, &this->two);
        model[strlen(model)-1]='\0';  //去掉末尾多输入的逗号
        this->type=string(model+1);

        vmType2Index[this->type]=vmType2Index.size();
    }
};

struct RequestInfo{
    int mode; //0add 1del
    int vmType; //虚拟机列表的下标，源输入是字符串型号
    int vmID;

    RequestInfo(){}
    RequestInfo(int mode,int vmType,int vmID){
        this->mode=mode;
        this->vmType=vmType;
        this->vmID=vmID;
    }

    void read_std(){
        static char add_del[6],buf[22];
        scanf("%s",add_del);
        if(strcmp(add_del,"(add,")==0)
        {
            scanf("%s",buf); buf[strlen(buf)-1]='\0'; //去掉逗号
            this->mode=0;
            this->vmType=vmType2Index[string(buf)];  //直接获得VM清单中的下标
        }
        else
        {
            this->mode=1; //del
        }
        scanf("%d)",&this->vmID);
    }
};
vector<ServerInfo> serverInfos;
vector<VMInfo> vmInfos;
vector<vector<RequestInfo>> requestInfos;

struct VM{
    int type;
    int ID;
    int serverIndex=-1;
    int node=-1;

    VM(){}
    VM(int vmType,int vmID){
        this->type=vmType;
        this->ID=vmID;
    }
    VM(int vmID,int serverIndex,int node){
        this->ID=vmID;
        this->serverIndex=serverIndex;
        this->node=node;
    }

    void setPutInfo(int serverIndex,int node){
        this->serverIndex=serverIndex;
        this->node=node;
    }

    int getCore(){
        return vmInfos[this->type].core;
    }
    int getMemory(){
        return vmInfos[this->type].memory;
    }
    int getTwo(){
        return vmInfos[this->type].two;
    }
};
unordered_map<int,VM>vmMap;
int alive_vm_count=0;


struct Server{
    int outputID = -1;
    int type;
    int coreA;
    int coreB;
    int memoryA;
    int memoryB;

    int vmNumsA=0; //每个节点上的虚拟机数量，初始为0
    int vmNumsB=0;
    int vm_one=0,vm_two=0; //单/双虚拟机的个数
    unordered_set<int>vms;

    Server(ServerInfo &ser){
        this->type = serverType2Index[ser.type];
        this->coreA = this->coreB = ser.core>>1;
        this->memoryA = this->memoryB = ser.memory>>1;
    }

    //给当前服务器分配编号
    void init_outputID(){
        static int SERVERID=0;
        this->outputID = SERVERID++;
    }

    int getACoreCap(){
        return serverInfos[this->type].core>>1;
    }

    int getAMemoryCap(){
        return serverInfos[this->type].memory>>1;
    }

    int getServerCost(){
        return serverInfos[this->type].serverCost;
    }

    int getPowerCost(){
        return serverInfos[this->type].powerCost;
    }

    double getDiff(){

        double diffA = (double)min(this->memoryA,this->coreA)/max(this->memoryA,this->coreA);
        double diffB = (double)min(this->memoryB,this->coreB)/max(this->memoryB,this->coreB);
        return min(diffA,diffB);

    }

    double getRestA(){
        double rMemoryA = (double)(this->memoryA)/this->getAMemoryCap();
        double rCoreA = (double)(this->coreA)/this->getACoreCap();
        return min(rCoreA,rMemoryA);
    }
    double getRestB(){
        double rMemoryB = (double)(this->memoryB)/this->getAMemoryCap();
        double rCoreB = (double)(this->coreB)/this->getACoreCap();
        return min(rCoreB,rMemoryB);
    }
    double getRest(){
        double rMemoryA = (double)(this->memoryA)/this->getAMemoryCap();
        double rMemoryB = (double)(this->memoryB)/this->getAMemoryCap();
        double rCoreA = (double)(this->coreA)/this->getACoreCap();
        double rCoreB = (double)(this->coreB)/this->getACoreCap();
        return max(min(rCoreA,rMemoryA),min(rCoreB,rMemoryB));
    }

    void putVM(struct VM &vmi,int serverIndex,int putNode)
    {
        vmi.setPutInfo(serverIndex,putNode);
        vmMap[vmi.ID]=vmi;
        this->vms.insert(vmi.ID);

        if(putNode!=2)this->vm_one++;
        else this->vm_two++;

        if(putNode == 0)
        {
            this->coreA-=vmi.getCore();
            this->memoryA-=vmi.getMemory();
            this->vmNumsA++;
        }
        else if (putNode == 1)
        {
            this->coreB-=vmi.getCore();
            this->memoryB-=vmi.getMemory();
            this->vmNumsB++;
        }
        else
        {
            this->coreA-=vmi.getCore()>>1;
            this->memoryA-=vmi.getMemory()>>1;
            this->vmNumsA++;
            this->coreB-=vmi.getCore()>>1;
            this->memoryB-=vmi.getMemory()>>1;
            this->vmNumsB++;
        }
    }

    //从当前服务器上删除vm
    void delVM(int vmID){
        struct VM &vmi=vmMap[vmID];
        //vmMap.erase(vmID);
        this->vms.erase(vmID);

        if(vmi.node!=2)this->vm_one--;
        else this->vm_two--;

        if(vmi.node==0){
            this->coreA += vmi.getCore();
            this->memoryA += vmi.getMemory();
            this->vmNumsA--;
        }else if(vmi.node==1){
            this->coreB += vmi.getCore();
            this->memoryB += vmi.getMemory();
            this->vmNumsB--;
        }else{
            this->coreA += vmi.getCore()>>1;
            this->memoryA += vmi.getMemory()>>1;
            this->vmNumsA--;
            this->coreB += vmi.getCore()>>1;
            this->memoryB += vmi.getMemory()>>1;
            this->vmNumsB--;
        }
    }
};
vector<Server>servers;


//初始化读入数据，但暂时不读入请求
int init()
{
    int n,m,totalDay, futureK;
    scanf("%d",&n);
    serverInfos.resize(n);
    for(auto &s:serverInfos)s.read_std();

    scanf("%d",&m);
    vmInfos.resize(m);
    for(auto &v:vmInfos)v.read_std();

    scanf("%d%d",&totalDay,&futureK);
    requestInfos.resize(totalDay); //初始化为总共d天
    return futureK;
}

//读入一天的请求，返回已读总天数
int getRequestsNextDay()
{
    static int day=0,tot;
    if(day==requestInfos.size())
        return day;
    scanf("%d",&tot);
    requestInfos[day].resize(tot);
    for(auto &rs:requestInfos[day])rs.read_std();
    return ++day;
}



//虚拟机选择放置位置，找不到则返回<-1,-1>
pair<int,int> chooseServer(VM &vmi,bool migration=false,int exactServerIndex=-1)
{
    int minRest=1<<30;
    int minRest_index = -1,putNode = -1;
    int restCore, restMem, rest;

    int start_i=0,end_i=servers.size();
    if(exactServerIndex!=-1) //指定服务器，即刚买的新服务器。
        start_i=exactServerIndex,end_i=exactServerIndex+1;
    for(int i=start_i;i<end_i; i++)//遍历服务器放置这台vm
    {
        Server &server=servers[i];

        //迁移时使用。考虑留在原位置不迁移时的代价
        if(migration && i==vmi.serverIndex)
        {
            if(vmi.node==0){
                rest = max(server.coreA, server.memoryA);
                if(rest < minRest){
                    minRest = rest;
                    minRest_index = i;
                    putNode = 0;
                }
                if (minRest <= 1) {  //有一个必等于0，一个小于等于1。
                    break;
                }

            }
            else if(vmi.node==1){
                rest = max(server.coreB, server.memoryB);
                if(rest < minRest){
                    minRest = rest;
                    minRest_index = i;
                    putNode = 1;
                }
                if (minRest <= 1) {
                    break;
                }
            }
            else if(vmi.node==2) {
                rest = min(max(server.coreA, server.memoryA), max(server.coreB, server.memoryB));
                if (rest < minRest) {
                    minRest = rest;
                    minRest_index = i;
                    putNode = 2;
                }
                if (minRest <= 1) {
                    break;
                }
            }
        }


        if(!vmi.getTwo())//单节点
        {
            if (server.coreA >= vmi.getCore() && server.memoryA >= vmi.getMemory()) {
                restCore = server.coreA - vmi.getCore();
                restMem = server.memoryA - vmi.getMemory();
                rest = max(restCore,restMem);
                if ((restCore <= Diff || restMem <= Diff) && abs(restMem - restCore) > diff_Nums) {
                    continue;
                }
                if (rest < minRest) {
                    minRest = rest;
                    minRest_index = i;
                    putNode = 0;
                }
                if (minRest <= 1) {  //有一个必等于0，一个小于等于1。
                    break;
                }
            }
            if (server.coreB >= vmi.getCore() && server.memoryB >= vmi.getMemory()) {
                restCore = server.coreB - vmi.getCore();
                restMem = server.memoryB - vmi.getMemory();
                rest = max(restCore,restMem);
                if ((restCore <= Diff || restMem <= Diff) && abs(restMem - restCore) > diff_Nums) {
                    continue;
                }
                if (rest < minRest) {
                    minRest = rest;
                    minRest_index = i;
                    putNode = 1;
                }
                if (minRest <= 1) {
                    break;
                }
            }
        }
        if(vmi.getTwo())//双节点
        {
            if (server.coreA >= vmi.getCore() / 2 && server.memoryA >= vmi.getMemory() / 2 && server.coreB >= vmi.getCore() / 2 && server.memoryB >= vmi.getMemory() / 2) {
                int restCoreA = server.coreA - vmi.getCore() / 2;
                int restMemA = server.memoryA - vmi.getMemory() / 2;
                int restCoreB = server.coreB - vmi.getCore() / 2;
                int restMemB = server.memoryB - vmi.getMemory() / 2;
                rest = min(max(restCoreA,restMemA),max(restCoreB,restMemB));
                if ((restCoreA <= Diff || restMemA <= Diff) && abs(restMemA - restCoreA) > diff_Nums) {
                    continue;
                }
                if ((restCoreB <= Diff || restMemB <= Diff) && abs(restMemB - restCoreB) > diff_Nums) {
                    continue;
                }
                if (rest < minRest) {
                    minRest = rest;
                    minRest_index = i;
                    putNode = 2;
                }
                if (minRest <= 1) {
                    break;
                }
            }
        }
    }
    return pair<int,int>(minRest_index,putNode);
}


//选择要购买的服务器
int choosePurchaseServer(VM &vmi,int today)
{
    int whatever = -1;
    int restDays=requestInfos.size()-today;

    //服务器清单排序
    static int serverIdx[102];
    for(int i=0;i<serverInfos.size();i++)serverIdx[i]=i;
    sort(serverIdx,serverIdx+serverInfos.size(),[restDays](int x,int y){
         ServerInfo &s1=serverInfos[x], &s2=serverInfos[y];
         return s1.serverCost+s1.powerCost*restDays < s2.serverCost+s2.powerCost*restDays;
    });

    //遍历服务器进行选购
    for(int i=0;i<serverInfos.size();i++)
    {
        ServerInfo &sInfo=serverInfos[serverIdx[i]];
        int restCore, restMemory;
        if(!vmi.getTwo())//单节点
        {
            restCore=(sInfo.core>>1)-vmi.getCore();
            restMemory=(sInfo.memory>>1)-vmi.getMemory();
        }
        else//双节点
        {
            restCore=(sInfo.core-vmi.getCore())>>1;
            restMemory=(sInfo.memory-vmi.getMemory())>>1;//这里的计算有待考虑!
        }

        if (restCore >= 0 && restMemory >= 0) {
            if ((restCore <= Diff || restMemory <= Diff) && abs(restCore - restMemory) > diff_Nums) {
                whatever = serverIdx[i];
                continue;
            }
            return serverIdx[i];
        }
    }
    return whatever;
}

/*
//选择要购买的服务器（打包状态）
int choosePurchaseServerWithPack(vector<int>cap,int today)
{
    int whatever = -1;
    int restDays=requestInfos.size()-today;

    //服务器清单排序
    static int serverIdx[102];
    for(int i=0;i<serverInfos.size();i++)serverIdx[i]=i;
    sort(serverIdx,serverIdx+serverInfos.size(),[restDays](int x,int y){
         ServerInfo &s1=serverInfos[x], &s2=serverInfos[y];
         return s1.serverCost+s1.powerCost*restDays < s2.serverCost+s2.powerCost*restDays;
    });

    //遍历服务器进行选购
    for(int i=0;i<serverInfos.size();i++)
    {
        ServerInfo &sInfo=serverInfos[serverIdx[i]];
        int restCore=sInfo.core/2-max(cap[0],cap[2]);
        int restMemory=sInfo.memory/2-max(cap[1],cap[3]);
        if (restCore >= 0 && restMemory >= 0) {
            if ((restCore <= Diff || restMemory <= Diff) && abs(restCore - restMemory) > diff_Nums) {
                whatever = serverIdx[i];
                continue;
            }
            return serverIdx[i];
        }
    }
    return whatever;
}
*/
//扩容(购买)服务器
unordered_map<int,int>& purchase(int today)
{
    static unordered_map<int,int>purchaseCount;//服务器清单下标->购买数量
    purchaseCount.clear();
    int purchaseNum=0; //今天新购服务器总数
    for(auto &req:requestInfos[today])  //遍历今天的请求
    {
        if(req.mode==0)//add
        {
            alive_vm_count++;
            struct VM vmi(req.vmType,req.vmID);//虚拟机实体
            //先尽量放虚拟机
            pair<int,int> chose=chooseServer(vmi);
            if(chose.first!=-1) //挑选到服务器了
            {
                servers[chose.first].putVM(vmi,chose.first,chose.second);
            }
            else //放置失败，那就买一台服务器吧
            {
                int serverInfoIndex = choosePurchaseServer(vmi,today);
                ServerInfo &newServer=serverInfos[serverInfoIndex];
                servers.push_back(Server(newServer));
                purchaseCount[serverInfoIndex]++;
                purchaseNum++;

                chose=chooseServer(vmi,false,servers.size()-1); //指定服务器编号为刚买的这台
                servers[chose.first].putVM(vmi,chose.first,chose.second);
            }
        }
        else //del
        {
            alive_vm_count--;
            servers[vmMap[req.vmID].serverIndex].delVM(req.vmID);
        }
    }

    //新购买的服务器分配编号，复杂度O(n*q)
    for(auto &info:purchaseCount)
    {
        for(int i=servers.size()-purchaseNum;i<servers.size();i++)
            if(info.first==servers[i].type)
                servers[i].init_outputID();
    }
    return purchaseCount;
}

/*
void doAdd(vector<VM>&addition,unordered_map<int,int>&purchaseCount,int &purchaseNum,int today)
{
    //先放置服务器
    for(VM &vmi:addition)
    {
        pair<int,int> chose=chooseServer(vmi);
        if(chose.first!=-1) //挑选到服务器了
        {
            servers[chose.first].putVM(vmi,chose.first,chose.second);
            vmi.ID=-1;//标记为已放置
        }
    }


    //放不下的VM，按单双节点分类
    vector<VM>one,two;
    for(VM &vmi:addition)if(vmi.ID!=-1)
    {
        if(vmi.getTwo())two.push_back(vmi);
        else one.push_back(vmi);
    }

    //双节点打包
    for(int i=0;i<two.size();)
    {
        //先尝试放进已有服务器
        pair<int,int> chose=chooseServer(two[i]);
        if(chose.first!=-1)
        {
            servers[chose.first].putVM(two[i],chose.first,chose.second);
            i++;
            continue;
        }

        //开始尝试打包购买服务器
        vector<int>cap({two[i].getCore()>>1, two[i].getMemory()>>1, two[i].getCore()>>1, two[i].getMemory()>>1});
        int preSid=choosePurchaseServerWithPack(cap,today); //万一打包失败，就按单独这一个VM买
        if(i+1<two.size())
        {
            cap[0]+=two[i+1].getCore()>>1;
            cap[1]+=two[i+1].getMemory()>>1;
            cap[2]+=two[i+1].getCore()>>1;
            cap[3]+=two[i+1].getMemory()>>1;
        }
        int packSid=choosePurchaseServerWithPack(cap,today);


        if(packSid==-1||i==two.size()-1)//打包失败，只按第一个VM买服务器
        {
            ServerInfo &newServer=serverInfos[preSid];
            servers.push_back(Server(newServer));
            purchaseCount[preSid]++;
            purchaseNum++;

            servers.back().putVM(two[i],servers.size()-1,2);
            i++;//前进一
        }
        else //打包成功
        {
            ServerInfo &newServer=serverInfos[packSid];
            servers.push_back(Server(newServer));
            purchaseCount[packSid]++;
            purchaseNum++;

            servers.back().putVM(two[i],servers.size()-1,2);
            servers.back().putVM(two[i+1],servers.size()-1,2);
            i+=2;//前进2
        }
    }

    //单节点打包
    for(int i=0;i<one.size();)
    {
        //先尝试放进已有服务器
        pair<int,int> chose=chooseServer(one[i]);
        if(chose.first!=-1)
        {
            servers[chose.first].putVM(one[i],chose.first,chose.second);
            i++;
            continue;
        }

        //开始尝试打包购买服务器
        vector<int>cap({one[i].getCore(), one[i].getMemory(), 0,0});
        int preSid=choosePurchaseServerWithPack(cap,today); //万一打包失败，就按单独为这个VM买服务器
        if(i+1<one.size())
        {
            cap[2]+=one[i+1].getCore();
            cap[3]+=one[i+1].getMemory();
        }
        int packSid=choosePurchaseServerWithPack(cap,today);

        if(true)
        //if(packSid==-1||i==one.size()-1)//打包失败，只按第一个VM买服务器
        {
            ServerInfo &newServer=serverInfos[preSid];
            servers.push_back(Server(newServer));
            purchaseCount[preSid]++;
            purchaseNum++;

            servers.back().putVM(one[i],servers.size()-1,1);
            i++;//前进一
        }
        else //打包成功
        {
            ServerInfo &newServer=serverInfos[packSid];
            servers.push_back(Server(newServer));
            purchaseCount[packSid]++;
            purchaseNum++;

            servers.back().putVM(one[i],servers.size()-1,0);
            servers.back().putVM(one[i+1],servers.size()-1,1);
            i+=2;//前进2
        }
    }
}

//扩容(购买)服务器
unordered_map<int,int>& purchaseWithPack(int today)
{
    static unordered_map<int,int>purchaseCount;//服务器清单下标->购买数量
    purchaseCount.clear();
    int purchaseNum=0; //今天新购服务器总数
    vector<VM>addition;
    for(auto &req:requestInfos[today])  //遍历今天的请求
    {
        if(req.mode==0)//add
        {
            alive_vm_count++;
            addition.push_back(VM(req.vmType,req.vmID));
        }
        else //del
        {
            alive_vm_count--;
            doAdd(addition,purchaseCount,purchaseNum,today);
            addition.clear();
            servers[vmMap[req.vmID].serverIndex].delVM(req.vmID);
        }
    }
    //处理今天剩余的VM
    doAdd(addition,purchaseCount,purchaseNum,today);

    //新购买的服务器分配编号，复杂度O(n*q)
    for(auto &info:purchaseCount)
    {
        for(int i=servers.size()-purchaseNum;i<servers.size();i++)
            if(info.first==servers[i].type)
                servers[i].init_outputID();
    }
    return purchaseCount;
}
*/
//迁移服务器
void migration(vector<VM>&migrationOut,int limit,int today)
{
    //已购服务器排序，该排序只为双节点虚拟机使用
    static int serverIdx[1<<20];
    for(int i=0;i<servers.size();i++)serverIdx[i]=i;
    sort(serverIdx,serverIdx+servers.size(),[](int x,int y){
         Server &s1=servers[x], &s2=servers[y];
         return s1.getRest() > s2.getRest();
    });

    //收集需要迁移的VM
    static vector<int> two,one;
    one.clear();
    two.clear();
    int permCount=0;
    for(int i=0;i<servers.size()&&permCount<limit*limit_num;i++) //遍历服务器
    {
        Server &tmpServer=servers[serverIdx[i]];
        for(int vmID:tmpServer.vms) //遍历当前服务器上的虚拟机
        {
            if(vmMap[vmID].getTwo())
            {
                two.push_back(vmID);
                permCount++;
            }
        }
    }

    //开始迁移  双节点
    int migrated=0;
    for(int &vmID:two)
    {
        if(migrated >= limit/2) break;
        VM &vmi=vmMap[vmID];
        pair<int,int> selected=chooseServer(vmi,true);  //寻找一台可跳槽的服务器，返回<server index,putting node>
        if(selected.first != vmi.serverIndex || selected.second != vmi.node)
        {
            servers[vmi.serverIndex].delVM(vmi.ID);
            servers[selected.first].putVM(vmi,selected.first,selected.second);//将vmi迁移到新服务器
            migrationOut.push_back(VM(vmi.ID,vmi.serverIndex,vmi.node));
            migrated++;
        }
    }

    ///********************服务器拆开，按节点排序**************************************************************************************
    for(int i=0;i<servers.size()*2;i++)serverIdx[i]=i; //偶数下标表示A节点
    sort(serverIdx,serverIdx+servers.size(),[](int x,int y){
         Server &s1=servers[x>>1], &s2=servers[y>>1];
         double rest1=((x&1)?s1.getRestB():s1.getRestA());
         double rest2=((y&1)?s2.getRestB():s2.getRestA());
         return rest1>rest2;
    });

    //收集需要迁移的VM
    permCount=0;
    for(int i=0;i<servers.size()*2&&permCount<limit*limit_num;i++) //遍历服务器
    {
        Server &tmpServer=servers[serverIdx[i]>>1];
        for(int vmID:tmpServer.vms) //遍历当前服务器上的虚拟机
        {
            if(vmMap[vmID].node==(serverIdx[i]&1)){
                one.push_back(vmID);
                permCount++;
            }
        }
    }

    //开始迁移  单节点
    for(int &vmID:one)
    {
        if(migrated >= limit) break;
        VM &vmi=vmMap[vmID];
        pair<int,int> selected=chooseServer(vmi,true);  //寻找一台可跳槽的服务器，返回<server index,putting node>
        if(selected.first != vmi.serverIndex || selected.second != vmi.node)
        {
            servers[vmi.serverIndex].delVM(vmi.ID);
            servers[selected.first].putVM(vmi,selected.first,selected.second);//将vmi迁移到新服务器
            migrationOut.push_back(VM(vmi.ID,vmi.serverIndex,vmi.node));
            migrated++;
        }
    }

}

int main()
{
    //freopen("../../../training-1.txt","r",stdin);
    //freopen("../../../training-1.out","w",stdout);
    int futureK = init(); //初始化读入
    for(int i=0;i<1;i++)getRequestsNextDay();
    for(int today=0;today<requestInfos.size();today++,getRequestsNextDay())
    {
        vector<VM> migrationOut;
        migration(migrationOut,alive_vm_count * 3 / 100,today);
      //  if(alive_vm_count * 3 / 100-migrationOut.size()>0)
      //      migration(migrationOut,alive_vm_count * 3 / 100-migrationOut.size(),today);

        unordered_map<int,int>& purchaseCount = purchase(today);

        //输出今天的方案
        printf("(purchase, %d)\n",purchaseCount.size());
        for(auto &s:purchaseCount)
            printf("(%s, %d)\n",serverInfos[s.first].type.c_str(),s.second);

        printf("(migration, %d)\n",migrationOut.size());
        for(auto &vmi:migrationOut)
        {
            printf("(%d, %d%s)\n",vmi.ID,servers[vmi.serverIndex].outputID, vmi.node==2?"":(vmi.node==0?", A":", B"));
        }

        for(auto &r:requestInfos[today])if(r.mode==0)//创建请求
        {
            auto &vmi=vmMap[r.vmID];
            printf("(%d%s)\n",servers[vmi.serverIndex].outputID,vmi.node==2?"":(vmi.node==0?", A":", B"));
        }
        fflush(stdout);
    }
    return 0;
}

