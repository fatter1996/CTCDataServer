#ifndef SYSUSER_H
#define SYSUSER_H
#include <QString>

//系统用户信息
class SysUser
{
public:
    SysUser();

public:
    //用户id
    int userId;
    //用户名
    QString userName;
    //登录名称
    QString loginName;
    //密码
    QString password;
    //盐加密
    QString salt;
    //车站id
    int staId;
    //站段id
    //long deptId;
    //用户角色
    int roleId;
    //登录状态，0未登录，1登录，2注销
    int loginStatus;
};

#endif // SYSUSER_H
