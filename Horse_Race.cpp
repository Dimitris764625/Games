#include<iostream>
#include<string>
#include<ctime>///two libraries needed to generate random numbers This is uneeded, you could just use srand(rand());
#include<cstdlib>
#include<cctype>
using namespace std;

int position(int pos, char horse)
{
cout << horse << ": ";
pos = pos + rand() % 2 + 1; /// this code will generate a random number between 1 and 2
for(int b=0;b<=pos;b++)
cout << " ";
cout << "~n-n^";
cout << endl << endl;
return pos;
}

int main()
{
int posA=0, posB=0, posC=0, posD=0;
char horse,horsename,bethorse;
double betcash=0;
cout<<"Who do you think is going to win?\nPlace your Bet and find out!\n> $";
cin>>betcash;
cout<<"On what horse? A, B, C, or D?\n> ";
cin>>bethorse;
if(bethorse=='a'){bethorse='A';}
if(bethorse=='b'){bethorse='B';}
if(bethorse=='c'){bethorse='C';}
if(bethorse=='d'){bethorse='D';}

srand(time(NULL)); /// this line of code ensures that our program will generate a new random number each time we ask for it

for(int a=0;a<=25;a++)
{
system("cls");

horse='A';
posA=position(posA,horse);
horse='B';
posB=position(posB,horse);
horse='C';
posC=position(posC,horse);
horse='D';
posD=position(posD,horse);
for(int timer=0;timer<=100000000;timer++);
}

int highest=0;
int winner[] = {posA, posB, posC, posD};


for(int p=0;p<=3;p++)
{
cout<<winner[p] << endl;
if(winner[p]>winner[highest]) ///Your Problem is here all answers are 2.
highest=p;
}
switch(highest){
case 0:
horsename='A';
break;
case 1:
horsename='B';
break;
case 2:
horsename='C';
break;
case 3:
horsename='D';
break;
}
cout<<"Winning Horse: Horse "<<horsename<<endl;
if(horsename==bethorse){cout<<"You Won! You Get triple times the money you paid."<<endl;}
else cout<<"You Lost! Now your $"<<betcash<<" that you just bet is now mine!"<<endl;

system("pause");
return 0;
}
