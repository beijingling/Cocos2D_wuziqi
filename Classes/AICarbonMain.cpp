#include "AICarbonMain.h"

#include "Carbon/AICarbon.h"


/*
 * AI 接口
 */

/* 全局变量 */

AICarbon* g_carbon;

/* 初始化 */
void Init() {
  g_carbon = new AICarbon;
}

/* 启动 */
void Start(int width, int height) {
  g_carbon ->start(width, height);
}

/* 落子 */
void Move(OXPiece type, int x, int y) {
  g_carbon->setWho(type);
  g_carbon->move(x, y);
}

/* 计算最佳位置 */
void AITurn(OXPiece type, int &x, int &y, int depth, int time) {
  g_carbon->setWho(type);
  g_carbon->yourTurn(x, y, depth, time);
}

/* 悔棋 */
void Undo() {
  g_carbon->undo();
  g_carbon->undo();
}

void StopAi() {
  terminateAI = 1;
}

void WriteLog(int points, int nSearched, int speed, int depth, bool debug)
{
  // do nothing
}
