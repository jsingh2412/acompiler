func : (cond : bool, cond2 : bool) int {
a : int;
a = 4;
a++;
a--;
a = a / 2;
if(a>1){
    a++;
}
if(a<1){
    a++;
} else {
    a--;
}
if(a==2){
    a = 5;
}
give "string";
give a;
if(!true){
b : int;
b--;
}
if(cond){
t: int;
t = 5;
}
else{
t: bool;
t = false;
}
while(cond2){
cond2 = false;
}
return a;
}
main : () int {
func(true, false);
magic: bool = 24Kmagic;
give magic;
}