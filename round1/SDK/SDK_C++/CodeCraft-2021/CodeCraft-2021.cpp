#include<bits/stdc++.h>
using namespace std;

#define DEBUG 0

//����
struct Request{
    bool mode; //1:add, 0:del
    string model; //������ͺ�
    int vmID;  //�����ID��������ָ��
};

//�����ʵ��
struct VM{
	int born,dead;
	string model;
	int server_idx, point;
};

//������ʵ��
struct Server{
	int ID; //ʵ��ID������ĿҪ������ķ�����ID
	string model;
	vector<int> cap;  //[remain cpu1,mem1,cpu2,mem2]

	unordered_set<int> vms[2]; //���������ڵ��ϸ��ص���������
	unordered_set<int> all_vms;
	int off_day=0;  //�ػ��գ������ǿ��ģ����Ϲأ�

	int vm_count(){
        return vms[0].size()+vms[1].size();
	}
	bool is_on(){
        return vms[0].size()>0 && vms[1].size()>0;
	}
};

unordered_map<string,vector<int>> serverList;  //�������嵥��model:[cpu1,mem1,cpu2,mem2,cost,daily]
unordered_map<string,vector<int>> VMList;  //������嵥��model:[cpu,mem,two?]
vector<vector<Request>> requests; //�����б�
unordered_map<int,VM> VMs; //���������
vector<Server> servers; //�ѹ�������

int max_cost=0,max_daycost=0;


//��ʼ��ȫ�ֱ���
void init()
{
	serverList.clear();
	VMList.clear();
	requests.clear();

	VMs.clear();
	servers.clear();
	max_cost=0,max_daycost=0;
}

//����������б�
void readServerList()
{
    int n,info[6];  //n<=100
    char model[22];
    scanf("%d",&n);
    while(n--)
    {
        scanf("%s%d,%d,%d,%d)",model,info,info+1,info+4,info+5);
        model[strlen(model)-1]='\0';  //ȥ��ĩβ������Ķ���
        info[0]=info[2]=info[0]/2;
        info[1]=info[3]=info[1]/2;
        serverList[string(model+1)]=vector<int>(info,info+6);
        max_cost=max(max_cost,info[4]);
        max_daycost=max(max_daycost,info[5]);
    }
}

//����������б�
void readVMList()
{
    int m,info[3];  //m<=1000
    char model[22];
    scanf("%d",&m);
    while(m--)
    {
        scanf("%s%d,%d,%d)",model,info,info+1,info+2);
        model[strlen(model)-1]='\0';  //ȥ��ĩβ������Ķ���
        VMList[string(model+1)]=vector<int>(info,info+3);
    }
}

//һ���Զ��������б�
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
		        r.model.pop_back();//ȥ������
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


//ͳ������������Ϣ
void statistics()
{
	for(int i=0;i<requests.size();i++)	//ͳ��ÿ��VM����������
		for(auto &r:requests[i])
		{
			if(r.mode)  //�������
			{
				VMs[r.vmID].born=i;
				VMs[r.vmID].dead=requests.size()-1;
				VMs[r.vmID].model=r.model;
			}
			else  //ɾ�����󣬸�������������ڽ�ֹ
			{
				VMs[r.vmID].dead=i;
			}
		}
}

//��ʾ��������������б�
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

//��ʾ�����б�
void printRequests()
{
	for(int i=0;i<requests.size();i++)
		for(auto &r:requests[i])
			printf("day %2d: %s\t%u\t%s\n",i,r.mode?"add":"del",r.vmID,r.model.c_str());
}

//��ʾ�ѹ���ķ�������Ϣ
void printServers()
{
	for(int i=0;i<servers.size();i++)
	{
		auto &cm=servers[i].cap;
		printf("%20s: %4d %4d %4d %4d\n",servers[i].model.c_str(),cm[0],cm[1],cm[2],cm[3]);
	}
}


int can_vm_into_server(int vmID,vector<int>&cap) //����vm��װ���ĸ��ڵ� return 0:failure, 1:A, 2:B, 3:AB˫�ڵ�
{
	vector<int> &vm=VMList[VMs[vmID].model];
	if(vm[2])//˫�ڵ�
	{
		if(vm[0]/2<=min(cap[0],cap[2]) && vm[1]/2<=min(cap[1],cap[3]))
			return 3;
		return 0;
	}
	//���ڵ�
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

void push_vm_into_server(int vmID,int server_idx,int point)  //װ��һ��VM
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

void del_vm_from_server(int vmID,int today) //ɾ�������
{
	vector<int> &vm=VMList[VMs[vmID].model];
	Server &s=servers[VMs[vmID].server_idx];
	if(VMs[vmID].point==3) //˫�ڵ�
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
	else //B���
	{
		s.cap[2]+=vm[0];
		s.cap[3]+=vm[1];
		s.vms[1].erase(vmID);
	}
	s.all_vms.erase(vmID);
	if(s.off_day==VMs[vmID].dead) //���¼���ػ���
	{
		int dead=today;
        for(auto &vid:s.all_vms)
            dead=max(dead,VMs[vid].dead);
		s.off_day=dead;
	}
}

//Ϊvm��һ̨��ʡǮ�ķ�����
pair<string,int> buy_server(string vm_model,int vm_ID, int remainDay)
{
	string &ret_model=vm_model;  //��㸳����ֵ����Ȼ����
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

//�����з�������ѡһ̨������
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

//Ǩ��
int idx[1<<19]={-1};
vector<vector<int>> migration(int mig_count,int today)
{
    vector<vector<int>> ret;

    if(idx[0]==-1)
        for(int i=0;i<(1<<18);i++)idx[i]=i;
    sort(idx,idx+servers.size(),[](int i,int j){
        Server &s1=servers[i],&s2=servers[j];
        if(s1.vm_count()!=s2.vm_count())
            return s1.vm_count()<s2.vm_count(); //����������ٵ����ȿ���Ǩ��
        return s1.cap[5]>s2.cap[5]; //�պĸߵ�����Ǩ��
    });


    unordered_set<int> ban_server;
    for(int i=0;i<servers.size() && ret.size()<mig_count && servers[idx[i]].vm_count()==1; i++)
    {
        ban_server.insert(idx[i]);
        auto all_vms=servers[idx[i]].all_vms;  //�൱�ڸ���һ��
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

        unordered_map<string,int>buyServer;//�����¹�������������Ϣ
		int buyServerCount=0; //�����¹�����������
		for(auto &r:requests[day])
		{
			if(r.mode)//��������
			{
			    alive_vm_count++;
			    pair<int,int>select= select_server(r.vmID, day);//��һ̨���з�����
				int server_id = select.first;
				int point = select.second;
				if(server_id==-1)  //���з�����װ���£�����һ̨�·�����
				{
					struct Server new_server;
					pair<string,int> buy=buy_server(r.model,r.vmID,requests.size()-day-1);
					new_server.model=buy.first;
					new_server.cap=vector<int>(serverList[new_server.model].begin(),serverList[new_server.model].begin()+6);
					servers.push_back(new_server);  //�·�����װ��������б�

					buyServer[new_server.model]++;
					buyServerCount++;

					server_id=servers.size()-1;
					point=buy.second;
				}
				push_vm_into_server(r.vmID,server_id,point);  //Ȼ���vmװ���������ڵ�
			}
			else //ɾ������
			{
			    alive_vm_count--;
				del_vm_from_server(r.vmID,day);
			}
		}

		//�����¹����������б��
		unordered_map<string,int>startIdx; //���ͺŷ�����ʵ����ʼ���
		int pre_count=servers.size()-buyServerCount;
		for(auto &v:buyServer)
		{
			startIdx[v.first]=pre_count;
			pre_count+=v.second;
		}
		for(int i=servers.size()-buyServerCount;i<servers.size();i++) //��Ҫ���б�ŵķ�����
		{
			servers[i].ID=startIdx[servers[i].model]++; //ʵ��ID
		}

        //�������ķ���
        printf("(purchase, %d)\n",buyServer.size());
        for(auto &v:buyServer)
            printf("(%s, %d)\n",v.first.c_str(),v.second);

        printf("(migration, %d)\n",mig.size());
        for(auto &v:mig)
        {
            printf("(%d, %d%s)\n",v[0],servers[v[1]].ID,v[2]==3?"":(v[2]==1?", A":", B"));
        }

        for(auto &r:requests[day])if(r.mode)//��������
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
        printf("===============��������Ϣ====================\n");
        printListInfos(serverList);
        printf("===============�������Ϣ====================\n");
        printListInfos(VMList);
        printf("===============������Ϣ======================\n");
        printRequests();

		printf("===============�������������================\n");
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
