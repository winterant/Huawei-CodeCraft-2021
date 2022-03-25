#include<bits/stdc++.h>
using namespace std;

#define DEBUG 0

//请求
struct Request{
    bool mode; //1:add, 0:del
    string model; //虚拟机型号
    int vmID;  //虚拟机ID，由输入指定
};

//虚拟机实体
struct VM{
	int born,dead;
	string model;
	int server_idx, point;
};

//服务器实体
struct Server{
	int ID; //实际ID，即题目要求输出的服务器ID
	string model;
	vector<int> cap;  //[remain cpu1,mem1,cpu2,mem2]

	unordered_set<int> vms[2]; //左右两个节点上负载的虚拟机编号
	unordered_set<int> all_vms;
	int off_day=0;  //关机日（当天是开的，晚上关）

	int vm_count(){
        return vms[0].size()+vms[1].size();
	}
	bool is_on(){
        return vms[0].size()>0 && vms[1].size()>0;
	}
};

unordered_map<string,vector<int>> serverList;  //服务器清单；model:[cpu1,mem1,cpu2,mem2,cost,daily]
unordered_map<string,vector<int>> VMList;  //虚拟机清单；model:[cpu,mem,two?]
vector<vector<Request>> requests; //需求列表
unordered_map<int,VM> VMs; //所有虚拟机
vector<Server> servers; //已购服务器

int max_cost=0,max_daycost=0;


//初始化全局变量
void init()
{
	serverList.clear();
	VMList.clear();
	requests.clear();

	VMs.clear();
	servers.clear();
	max_cost=0,max_daycost=0;
}

//读入服务器列表
void readServerList()
{
    int n,info[6];  //n<=100
    char model[22];
    scanf("%d",&n);
    while(n--)
    {
        scanf("%s%d,%d,%d,%d)",model,info,info+1,info+4,info+5);
        model[strlen(model)-1]='\0';  //去掉末尾多输入的逗号
        info[0]=info[2]=info[0]/2;
        info[1]=info[3]=info[1]/2;
        serverList[string(model+1)]=vector<int>(info,info+6);
        max_cost=max(max_cost,info[4]);
        max_daycost=max(max_daycost,info[5]);
    }
}

//读入虚拟机列表
void readVMList()
{
    int m,info[3];  //m<=1000
    char model[22];
    scanf("%d",&m);
    while(m--)
    {
        scanf("%s%d,%d,%d)",model,info,info+1,info+2);
        model[strlen(model)-1]='\0';  //去掉末尾多输入的逗号
        VMList[string(model+1)]=vector<int>(info,info+3);
    }
}

//一次性读入请求列表
void readRequests()
{
    int sumday,perday;
	char mode[6],buf[22];
    Request r;
    scanf("%d",&sumday);
    for(int i=0;i<sumday;i++)
    {
        scanf("%d",&perday);
    	requests.push_back(vector<Request>()); //new day
        for(int j=0;j<perday;j++)
        {
            scanf("%s",mode);
		    if(strcmp(mode,"(add,")==0)
		    {
		        scanf("%s",buf);
		        r.mode=true;
		        r.model=string(buf);
		        r.model.pop_back();//去掉逗号
		    }
		    else
		    {
		        r.mode=false; //del
		        r.model=string("");
		    }
		    scanf("%d)",&r.vmID);
            requests.back().push_back(r);
        }
    }
}


//统计请求的相关信息
void statistics()
{
	for(int i=0;i<requests.size();i++)	//统计每个VM的生命周期
		for(auto &r:requests[i])
		{
			if(r.mode)  //添加请求
			{
				VMs[r.vmID].born=i;
				VMs[r.vmID].dead=requests.size()-1;
				VMs[r.vmID].model=r.model;
			}
			else  //删除请求，该虚拟机生命周期截止
			{
				VMs[r.vmID].dead=i;
			}
		}
}

//显示服务器或虚拟机列表
void printListInfos(unordered_map<string,vector<int>> List)
{
    for(auto &i:List)
    {
        printf("%s",i.first.c_str());
        for(auto &v:i.second)
            printf("\t%7d",v);
        printf("\n");
    }
}

//显示请求列表
void printRequests()
{
	for(int i=0;i<requests.size();i++)
		for(auto &r:requests[i])
			printf("day %2d: %s\t%u\t%s\n",i,r.mode?"add":"del",r.vmID,r.model.c_str());
}

//显示已购买的服务器信息
void printServers()
{
	for(int i=0;i<servers.size();i++)
	{
		auto &cm=servers[i].cap;
		printf("%20s: %4d %4d %4d %4d\n",servers[i].model.c_str(),cm[0],cm[1],cm[2],cm[3]);
	}
}


int can_vm_into_server(int vmID,vector<int>&cap) //计算vm能装进哪个节点 return 0:failure, 1:A, 2:B, 3:AB双节点
{
	vector<int> &vm=VMList[VMs[vmID].model];
	if(vm[2])//双节点
	{
		if(vm[0]/2<=min(cap[0],cap[2]) && vm[1]/2<=min(cap[1],cap[3]))
			return 3;
		return 0;
	}
	//单节点
	if(cap[0]+cap[1]>=cap[2]+cap[3])
	{
		if(vm[0]<=cap[0] && vm[1]<=cap[1])
			return 1;
		if(vm[0]<=cap[2] && vm[1]<=cap[3])
			return 2;
	}else{
		if(vm[0]<=cap[2] && vm[1]<=cap[3])
			return 2;
		if(vm[0]<=cap[0] && vm[1]<=cap[1])
			return 1;
	}
	return 0;
}

void push_vm_into_server(int vmID,int server_idx,int point)  //装入一个VM
{
	VMs[vmID].server_idx=server_idx;
	VMs[vmID].point=point;
	vector<int> &vm=VMList[VMs[vmID].model];
	Server &s=servers[server_idx];
	if(point==1)
	{
		s.cap[0]-=vm[0];
		s.cap[1]-=vm[1];
		s.vms[0].insert(vmID);
	}
	if(point==2)
	{
		s.cap[2]-=vm[0];
		s.cap[3]-=vm[1];
		s.vms[1].insert(vmID);
	}
	if(point==3)
	{
		s.cap[0]-=vm[0]/2;
		s.cap[1]-=vm[1]/2;
		s.cap[2]-=vm[0]/2;
		s.cap[3]-=vm[1]/2;
		s.vms[0].insert(vmID);
		s.vms[1].insert(vmID);
	}
	s.all_vms.insert(vmID);
	if(s.off_day<VMs[vmID].dead)
		s.off_day=VMs[vmID].dead;
}

void del_vm_from_server(int vmID,int today) //删除虚拟机
{
	vector<int> &vm=VMList[VMs[vmID].model];
	Server &s=servers[VMs[vmID].server_idx];
	if(VMs[vmID].point==3) //双节点
	{
		s.cap[0]+=vm[0]/2;
		s.cap[1]+=vm[1]/2;
		s.cap[2]+=vm[0]/2;
		s.cap[3]+=vm[1]/2;
		s.vms[0].erase(vmID);
		s.vms[1].erase(vmID);
	}
	else if(VMs[vmID].point==1)
	{
		s.cap[0]+=vm[0];
		s.cap[1]+=vm[1];
		s.vms[0].erase(vmID);
	}
	else //B结点
	{
		s.cap[2]+=vm[0];
		s.cap[3]+=vm[1];
		s.vms[1].erase(vmID);
	}
	s.all_vms.erase(vmID);
	if(s.off_day==VMs[vmID].dead) //重新计算关机日
	{
		int dead=today;
        for(auto &vid:s.all_vms)
            dead=max(dead,VMs[vid].dead);
		s.off_day=dead;
	}
}

//为vm买一台最省钱的服务器
pair<string,int> buy_server(string vm_model,int vm_ID, int remainDay)
{
	string &ret_model=vm_model;  //随便赋个初值，不然报错
	auto &vm=VMList[vm_model];
	double min_cost=1e18;
	for(auto &v:serverList)if(can_vm_into_server(vm_ID,v.second))
	{
		double cur_cost=v.second[4]*1.0/max_cost
				+v.second[5]*remainDay*1.0/max_daycost
				+0.01*fabs(vm[0]*1.0/vm[1] - v.second[0]*1.0/v.second[1]);
		if(min_cost>cur_cost)
		{
			min_cost=cur_cost;
			ret_model=v.first;
		}
	}
	return pair<string,int>(ret_model,vm[2]?3:1);
}

//从已有服务器挑选一台服务器
pair<int,int> select_server(int vmID,int today,unordered_set<int> ban_server_id=unordered_set<int>())
{
	pair<int,int>ret(-1,-1);
	double min_daycost=1e18;
	for(int i=0;i<servers.size();i++)if(!ban_server_id.count(i))
	{
		int point=can_vm_into_server(vmID,servers[i].cap);
		if(!point)continue;

		Server &s=servers[i];
		int start_day = (s.is_on() ? s.off_day : today);
		double daycost=(VMs[vmID].dead-start_day)*s.cap[5];

		if(min_daycost>daycost)
		{
			min_daycost=daycost;
			ret.first=i;
			ret.second=point;
		}
	}
	return ret;
}

//迁移
int idx[1<<19]={-1};
vector<vector<int>> migration(int mig_count,int today)
{
    vector<vector<int>> ret;

    if(idx[0]==-1)
        for(int i=0;i<(1<<18);i++)idx[i]=i;
    sort(idx,idx+servers.size(),[](int i,int j){
        Server &s1=servers[i],&s2=servers[j];
        if(s1.vm_count()!=s2.vm_count())
            return s1.vm_count()<s2.vm_count(); //负载虚拟机少的优先考虑迁移
        return s1.cap[5]>s2.cap[5]; //日耗高的优先迁移
    });


    unordered_set<int> ban_server;
    for(int i=0;i<servers.size() && ret.size()<mig_count && servers[idx[i]].vm_count()==1; i++)
    {
        ban_server.insert(idx[i]);
        auto all_vms=servers[idx[i]].all_vms;  //相当于复制一份
        for(auto &vmID:all_vms)
        {
            pair<int,int> select=select_server(vmID,today,ban_server);
            if(select.first!=-1)
            {
                del_vm_from_server(vmID,today);
                push_vm_into_server(vmID,select.first,select.second);
                ret.push_back(vector<int>({vmID,select.first,select.second}));
            }
            if(ret.size()>=mig_count)break;
        }
        if(servers[idx[i]].is_on())
            ban_server.erase(idx[i]);
    }
    return ret;
}

void process()
{
    int alive_vm_count=0;
	for(int day=0;day<requests.size();day++)
	{
	    vector<vector<int>> mig = migration(alive_vm_count/200,day);

        unordered_map<string,int>buyServer;//今天新购服务器购买信息
		int buyServerCount=0; //今天新购服务器总数
		for(auto &r:requests[day])
		{
			if(r.mode)//创建请求
			{
			    alive_vm_count++;
			    pair<int,int>select= select_server(r.vmID, day);//找一台已有服务器
				int server_id = select.first;
				int point = select.second;
				if(server_id==-1)  //已有服务器装不下，先买一台新服务器
				{
					struct Server new_server;
					pair<string,int> buy=buy_server(r.model,r.vmID,requests.size()-day-1);
					new_server.model=buy.first;
					new_server.cap=vector<int>(serverList[new_server.model].begin(),serverList[new_server.model].begin()+6);
					servers.push_back(new_server);  //新服务器装入服务器列表

					buyServer[new_server.model]++;
					buyServerCount++;

					server_id=servers.size()-1;
					point=buy.second;
				}
				push_vm_into_server(r.vmID,server_id,point);  //然后把vm装进服务器节点
			}
			else //删除请求
			{
			    alive_vm_count--;
				del_vm_from_server(r.vmID,day);
			}
		}

		//今日新购服务器进行编号
		unordered_map<string,int>startIdx; //该型号服务器实际起始编号
		int pre_count=servers.size()-buyServerCount;
		for(auto &v:buyServer)
		{
			startIdx[v.first]=pre_count;
			pre_count+=v.second;
		}
		for(int i=servers.size()-buyServerCount;i<servers.size();i++) //需要进行编号的服务器
		{
			servers[i].ID=startIdx[servers[i].model]++; //实际ID
		}

        //输出今天的方案
        printf("(purchase, %d)\n",buyServer.size());
        for(auto &v:buyServer)
            printf("(%s, %d)\n",v.first.c_str(),v.second);

        printf("(migration, %d)\n",mig.size());
        for(auto &v:mig)
        {
            printf("(%d, %d%s)\n",v[0],servers[v[1]].ID,v[2]==3?"":(v[2]==1?", A":", B"));
        }

        for(auto &r:requests[day])if(r.mode)//创建请求
        {
            auto &v=VMs[r.vmID];
            printf("(%d%s)\n",servers[v.server_idx].ID,v.point==3?"":(v.point==1?", A":", B"));
        }
	}
}

void work()
{
	init();

    readServerList();
    readVMList();
    readRequests();
	statistics();
    /*
    #if DEBUG
        printf("===============服务器信息====================\n");
        printListInfos(serverList);
        printf("===============虚拟机信息====================\n");
        printListInfos(VMList);
        printf("===============需求信息======================\n");
        printRequests();

		printf("===============虚拟机生命周期================\n");
		for(auto &v:VMsLife)
			printf("%d : life=%d\t%s\n",v.first,v.second,VMsModel[v.first].c_str());
	#endif
	*/
    process();
}

int main()
{
    //freopen("../../../training-data/sample.txt","r",stdin);
    //freopen("../../../training-data/training-1.txt","r",stdin);
    //freopen("../../../training-data/training-1.out","w",stdout);
    work();
}
