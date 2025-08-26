# 텔레칩스 임베디드스쿨 3기 게임 제작 프로젝트
## Tank Boy
### 게임 시나리오
* 탱크(게임자)가 맵을 이동하며 맵에 존재하는 장애물과 적들의 공격을 피하며 생존한다. 
* 장애물과 적에게 공격당하면 생명이 줄어든다.
* 탱크는 기관총과 대포로 적을 제거할 수 있으며 스테이지가 넘어갈 때마다 다양한 적들이 나타나며 총알 속도, 이동속도, 체력, 수가 증가한다.
* 제거한 적의 난이도에 따른 점수가 측정되고, 많은 적을 죽이며 높은 난이도까지 올라갈수록 점수가 높아진다.


### vscode 환경구성 ()
```sh
winget install Microsoft.VisualStudio.2022.BuildTools

# build tutorial game
build_example.bat

# launch tutorial game
cd example_tutorial/game_tutorial
x64\Debug\game_tutorial.exe
```