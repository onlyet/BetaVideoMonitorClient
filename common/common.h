#ifndef COMMON_H
#define COMMON_H

// ±£´æÂ·¾¶
#define LeftRecordPath(rootPath, middlePath, userId)        QString("%1/%2/%3_left.ts").arg(rootPath).arg(middlePath).arg(userId)
#define RightRecordPath(rootPath, middlePath, userId)       QString("%1/%2/%3_right.ts").arg(rootPath).arg(middlePath).arg(userId)

#endif // COMMON_H
