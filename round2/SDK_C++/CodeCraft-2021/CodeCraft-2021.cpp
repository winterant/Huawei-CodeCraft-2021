#include<bits/stdc++.h>
#include "parameters.h"
using namespace std;



unordered_map<string,int> serverType2Index;  //�������ͺ�->�嵥�ϵı��
unordered_map<string,int> vmType2Index;  //������ͺ�->�嵥�ϵı��

struct ServerInfo{
    string type;
    int core;
    int memory;
    int serverCost;
    int powerCost;

    void read_std(){
        static char model[22];
        scanf("%s%d,%d,%d,%d)",model,&this->core, &this->memory, &this->serverCost,&this->powerCost);
        model[strlen(model)-1]='\0';  //ȥ��ĩβ������Ķ���
        this->type=string(model+1);  //modelȥ���׸��ַ�������

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
        model[strlen(model)-1]='\0';  //ȥ��ĩβ������Ķ���
        this->type=string(model+1);

        vmType2Index[this->type]=vmType2Index.size();
    }
};

struct RequestInfo{
    int mode; //0add 1del
    int vmType; //������б���±꣬Դ�������ַ����ͺ�
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
            scanf("%s",buf); buf[strlen(buf)-1]='\0'; //ȥ������
            this->mode=0;
            this->vmType=vmType2Index[string(buf)];  //ֱ�ӻ��VM�嵥�е��±�
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

    int vmNumsA=0; //ÿ���ڵ��ϵ��������������ʼΪ0
    int vmNumsB=0;
    int vm_one=0,vm_two=0; //��/˫������ĸ���
    unordered_set<int>vms;

    Server(ServerInfo &ser){
        this->type = serverType2Index[ser.type];
        this->coreA = this->coreB = ser.core>>1;
        this->memoryA = this->memoryB = ser.memory>>1;
    }

    //����ǰ������������
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

    //�ӵ�ǰ��������ɾ��vm
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


//��ʼ���������ݣ�����ʱ����������
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
    requestInfos.resize(totalDay); //��ʼ��Ϊ�ܹ�d��
    return futureK;
}

//����һ������󣬷����Ѷ�������
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



//�����ѡ�����λ�ã��Ҳ����򷵻�<-1,-1>
pair<int,int> chooseServer(VM &vmi,bool migration=false,int exactServerIndex=-1)
{
    int minRest=1<<30;
    int minRest_index = -1,putNode = -1;
    int restCore, restMem, rest;

    int start_i=0,end_i=servers.size();
    if(exactServerIndex!=-1) //ָ������������������·�������
        start_i=exactServerIndex,end_i=exactServerIndex+1;
    for(int i=start_i;i<end_i; i++)//����������������̨vm
    {
        Server &server=servers[i];

        //Ǩ��ʱʹ�á���������ԭλ�ò�Ǩ��ʱ�Ĵ���
        if(migration && i==vmi.serverIndex)
        {
            if(vmi.node==0){
                rest = max(server.coreA, server.memoryA);
                if(rest < minRest){
                    minRest = rest;
                    minRest_index = i;
                    putNode = 0;
                }
                if (minRest <= 1) {  //��һ���ص���0��һ��С�ڵ���1��
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


        if(!vmi.getTwo())//���ڵ�
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
                if (minRest <= 1) {  //��һ���ص���0��һ��С�ڵ���1��
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
        if(vmi.getTwo())//˫�ڵ�
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


//ѡ��Ҫ����ķ�����
int choosePurchaseServer(VM &vmi,int today)
{
    int whatever = -1;
    int restDays=requestInfos.size()-today;

    //�������嵥����
    static int serverIdx[102];
    for(int i=0;i<serverInfos.size();i++)serverIdx[i]=i;
    sort(serverIdx,serverIdx+serverInfos.size(),[restDays](int x,int y){
         ServerInfo &s1=serverInfos[x], &s2=serverInfos[y];
         return s1.serverCost+s1.powerCost*restDays < s2.serverCost+s2.powerCost*restDays;
    });

    //��������������ѡ��
    for(int i=0;i<serverInfos.size();i++)
    {
        ServerInfo &sInfo=serverInfos[serverIdx[i]];
        int restCore, restMemory;
        if(!vmi.getTwo())//���ڵ�
        {
            restCore=(sInfo.core>>1)-vmi.getCore();
            restMemory=(sInfo.memory>>1)-vmi.getMemory();
        }
        else//˫�ڵ�
        {
            restCore=(sInfo.core-vmi.getCore())>>1;
            restMemory=(sInfo.memory-vmi.getMemory())>>1;//����ļ����д�����!
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
//ѡ��Ҫ����ķ����������״̬��
int choosePurchaseServerWithPack(vector<int>cap,int today)
{
    int whatever = -1;
    int restDays=requestInfos.size()-today;

    //�������嵥����
    static int serverIdx[102];
    for(int i=0;i<serverInfos.size();i++)serverIdx[i]=i;
    sort(serverIdx,serverIdx+serverInfos.size(),[restDays](int x,int y){
         ServerInfo &s1=serverInfos[x], &s2=serverInfos[y];
         return s1.serverCost+s1.powerCost*restDays < s2.serverCost+s2.powerCost*restDays;
    });

    //��������������ѡ��
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
//����(����)������
unordered_map<int,int>& purchase(int today)
{
    static unordered_map<int,int>purchaseCount;//�������嵥�±�->��������
    purchaseCount.clear();
    int purchaseNum=0; //�����¹�����������
    for(auto &req:requestInfos[today])  //�������������
    {
        if(req.mode==0)//add
        {
            alive_vm_count++;
            struct VM vmi(req.vmType,req.vmID);//�����ʵ��
            //�Ⱦ����������
            pair<int,int> chose=chooseServer(vmi);
            if(chose.first!=-1) //��ѡ����������
            {
                servers[chose.first].putVM(vmi,chose.first,chose.second);
            }
            else //����ʧ�ܣ��Ǿ���һ̨��������
            {
                int serverInfoIndex = choosePurchaseServer(vmi,today);
                ServerInfo &newServer=serverInfos[serverInfoIndex];
                servers.push_back(Server(newServer));
                purchaseCount[serverInfoIndex]++;
                purchaseNum++;

                chose=chooseServer(vmi,false,servers.size()-1); //ָ�����������Ϊ�������̨
                servers[chose.first].putVM(vmi,chose.first,chose.second);
            }
        }
        else //del
        {
            alive_vm_count--;
            servers[vmMap[req.vmID].serverIndex].delVM(req.vmID);
        }
    }

    //�¹���ķ����������ţ����Ӷ�O(n*q)
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
    //�ȷ��÷�����
    for(VM &vmi:addition)
    {
        pair<int,int> chose=chooseServer(vmi);
        if(chose.first!=-1) //��ѡ����������
        {
            servers[chose.first].putVM(vmi,chose.first,chose.second);
            vmi.ID=-1;//���Ϊ�ѷ���
        }
    }


    //�Ų��µ�VM������˫�ڵ����
    vector<VM>one,two;
    for(VM &vmi:addition)if(vmi.ID!=-1)
    {
        if(vmi.getTwo())two.push_back(vmi);
        else one.push_back(vmi);
    }

    //˫�ڵ���
    for(int i=0;i<two.size();)
    {
        //�ȳ��ԷŽ����з�����
        pair<int,int> chose=chooseServer(two[i]);
        if(chose.first!=-1)
        {
            servers[chose.first].putVM(two[i],chose.first,chose.second);
            i++;
            continue;
        }

        //��ʼ���Դ�����������
        vector<int>cap({two[i].getCore()>>1, two[i].getMemory()>>1, two[i].getCore()>>1, two[i].getMemory()>>1});
        int preSid=choosePurchaseServerWithPack(cap,today); //��һ���ʧ�ܣ��Ͱ�������һ��VM��
        if(i+1<two.size())
        {
            cap[0]+=two[i+1].getCore()>>1;
            cap[1]+=two[i+1].getMemory()>>1;
            cap[2]+=two[i+1].getCore()>>1;
            cap[3]+=two[i+1].getMemory()>>1;
        }
        int packSid=choosePurchaseServerWithPack(cap,today);


        if(packSid==-1||i==two.size()-1)//���ʧ�ܣ�ֻ����һ��VM�������
        {
            ServerInfo &newServer=serverInfos[preSid];
            servers.push_back(Server(newServer));
            purchaseCount[preSid]++;
            purchaseNum++;

            servers.back().putVM(two[i],servers.size()-1,2);
            i++;//ǰ��һ
        }
        else //����ɹ�
        {
            ServerInfo &newServer=serverInfos[packSid];
            servers.push_back(Server(newServer));
            purchaseCount[packSid]++;
            purchaseNum++;

            servers.back().putVM(two[i],servers.size()-1,2);
            servers.back().putVM(two[i+1],servers.size()-1,2);
            i+=2;//ǰ��2
        }
    }

    //���ڵ���
    for(int i=0;i<one.size();)
    {
        //�ȳ��ԷŽ����з�����
        pair<int,int> chose=chooseServer(one[i]);
        if(chose.first!=-1)
        {
            servers[chose.first].putVM(one[i],chose.first,chose.second);
            i++;
            continue;
        }

        //��ʼ���Դ�����������
        vector<int>cap({one[i].getCore(), one[i].getMemory(), 0,0});
        int preSid=choosePurchaseServerWithPack(cap,today); //��һ���ʧ�ܣ��Ͱ�����Ϊ���VM�������
        if(i+1<one.size())
        {
            cap[2]+=one[i+1].getCore();
            cap[3]+=one[i+1].getMemory();
        }
        int packSid=choosePurchaseServerWithPack(cap,today);

        if(true)
        //if(packSid==-1||i==one.size()-1)//���ʧ�ܣ�ֻ����һ��VM�������
        {
            ServerInfo &newServer=serverInfos[preSid];
            servers.push_back(Server(newServer));
            purchaseCount[preSid]++;
            purchaseNum++;

            servers.back().putVM(one[i],servers.size()-1,1);
            i++;//ǰ��һ
        }
        else //����ɹ�
        {
            ServerInfo &newServer=serverInfos[packSid];
            servers.push_back(Server(newServer));
            purchaseCount[packSid]++;
            purchaseNum++;

            servers.back().putVM(one[i],servers.size()-1,0);
            servers.back().putVM(one[i+1],servers.size()-1,1);
            i+=2;//ǰ��2
        }
    }
}

//����(����)������
unordered_map<int,int>& purchaseWithPack(int today)
{
    static unordered_map<int,int>purchaseCount;//�������嵥�±�->��������
    purchaseCount.clear();
    int purchaseNum=0; //�����¹�����������
    vector<VM>addition;
    for(auto &req:requestInfos[today])  //�������������
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
    //�������ʣ���VM
    doAdd(addition,purchaseCount,purchaseNum,today);

    //�¹���ķ����������ţ����Ӷ�O(n*q)
    for(auto &info:purchaseCount)
    {
        for(int i=servers.size()-purchaseNum;i<servers.size();i++)
            if(info.first==servers[i].type)
                servers[i].init_outputID();
    }
    return purchaseCount;
}
*/
//Ǩ�Ʒ�����
void migration(vector<VM>&migrationOut,int limit,int today)
{
    //�ѹ����������򣬸�����ֻΪ˫�ڵ������ʹ��
    static int serverIdx[1<<20];
    for(int i=0;i<servers.size();i++)serverIdx[i]=i;
    sort(serverIdx,serverIdx+servers.size(),[](int x,int y){
         Server &s1=servers[x], &s2=servers[y];
         return s1.getRest() > s2.getRest();
    });

    //�ռ���ҪǨ�Ƶ�VM
    static vector<int> two,one;
    one.clear();
    two.clear();
    int permCount=0;
    for(int i=0;i<servers.size()&&permCount<limit*limit_num;i++) //����������
    {
        Server &tmpServer=servers[serverIdx[i]];
        for(int vmID:tmpServer.vms) //������ǰ�������ϵ������
        {
            if(vmMap[vmID].getTwo())
            {
                two.push_back(vmID);
                permCount++;
            }
        }
    }

    //��ʼǨ��  ˫�ڵ�
    int migrated=0;
    for(int &vmID:two)
    {
        if(migrated >= limit/2) break;
        VM &vmi=vmMap[vmID];
        pair<int,int> selected=chooseServer(vmi,true);  //Ѱ��һ̨�����۵ķ�����������<server index,putting node>
        if(selected.first != vmi.serverIndex || selected.second != vmi.node)
        {
            servers[vmi.serverIndex].delVM(vmi.ID);
            servers[selected.first].putVM(vmi,selected.first,selected.second);//��vmiǨ�Ƶ��·�����
            migrationOut.push_back(VM(vmi.ID,vmi.serverIndex,vmi.node));
            migrated++;
        }
    }

    ///********************�������𿪣����ڵ�����**************************************************************************************
    for(int i=0;i<servers.size()*2;i++)serverIdx[i]=i; //ż���±��ʾA�ڵ�
    sort(serverIdx,serverIdx+servers.size(),[](int x,int y){
         Server &s1=servers[x>>1], &s2=servers[y>>1];
         double rest1=((x&1)?s1.getRestB():s1.getRestA());
         double rest2=((y&1)?s2.getRestB():s2.getRestA());
         return rest1>rest2;
    });

    //�ռ���ҪǨ�Ƶ�VM
    permCount=0;
    for(int i=0;i<servers.size()*2&&permCount<limit*limit_num;i++) //����������
    {
        Server &tmpServer=servers[serverIdx[i]>>1];
        for(int vmID:tmpServer.vms) //������ǰ�������ϵ������
        {
            if(vmMap[vmID].node==(serverIdx[i]&1)){
                one.push_back(vmID);
                permCount++;
            }
        }
    }

    //��ʼǨ��  ���ڵ�
    for(int &vmID:one)
    {
        if(migrated >= limit) break;
        VM &vmi=vmMap[vmID];
        pair<int,int> selected=chooseServer(vmi,true);  //Ѱ��һ̨�����۵ķ�����������<server index,putting node>
        if(selected.first != vmi.serverIndex || selected.second != vmi.node)
        {
            servers[vmi.serverIndex].delVM(vmi.ID);
            servers[selected.first].putVM(vmi,selected.first,selected.second);//��vmiǨ�Ƶ��·�����
            migrationOut.push_back(VM(vmi.ID,vmi.serverIndex,vmi.node));
            migrated++;
        }
    }

}

int main()
{
    //freopen("../../../training-1.txt","r",stdin);
    //freopen("../../../training-1.out","w",stdout);
    int futureK = init(); //��ʼ������
    for(int i=0;i<1;i++)getRequestsNextDay();
    for(int today=0;today<requestInfos.size();today++,getRequestsNextDay())
    {
        vector<VM> migrationOut;
        migration(migrationOut,alive_vm_count * 3 / 100,today);
      //  if(alive_vm_count * 3 / 100-migrationOut.size()>0)
      //      migration(migrationOut,alive_vm_count * 3 / 100-migrationOut.size(),today);

        unordered_map<int,int>& purchaseCount = purchase(today);

        //�������ķ���
        printf("(purchase, %d)\n",purchaseCount.size());
        for(auto &s:purchaseCount)
            printf("(%s, %d)\n",serverInfos[s.first].type.c_str(),s.second);

        printf("(migration, %d)\n",migrationOut.size());
        for(auto &vmi:migrationOut)
        {
            printf("(%d, %d%s)\n",vmi.ID,servers[vmi.serverIndex].outputID, vmi.node==2?"":(vmi.node==0?", A":", B"));
        }

        for(auto &r:requestInfos[today])if(r.mode==0)//��������
        {
            auto &vmi=vmMap[r.vmID];
            printf("(%d%s)\n",servers[vmi.serverIndex].outputID,vmi.node==2?"":(vmi.node==0?", A":", B"));
        }
        fflush(stdout);
    }
    return 0;
}

