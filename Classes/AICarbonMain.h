#ifndef CARBON_AI_MAIN_H
#define CARBON_AI_MAIN_H

#include "Carbon/OXTypes.h"

/*
 * AI 接口
 */
/* 初始化 */
void Init();

/* 启动 */
void Start(int width, int height);

/* 落子 */
void Move(OXPiece type, int x, int y);

/* 计算最佳位置 */
void AITurn(OXPiece type, int &x, int &y, int depth = 0, int time = 0);

/* 悔棋 */
void Undo();

void StopAi();

void WriteLog(int points, int nSearched, int speed, int depth, bool debug);
#endif // CARBON_AI_MAIN_H
