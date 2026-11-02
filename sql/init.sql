-- ============================================================
-- 三期网盘初始化脚本
-- 使用：mysql -uroot -p < sql/init.sql
-- ============================================================

DROP DATABASE IF EXISTS cloud_storage;
CREATE DATABASE cloud_storage
    DEFAULT CHARACTER SET utf8mb4
    DEFAULT COLLATE utf8mb4_unicode_ci;

USE cloud_storage;

-- ------------------------------------------------------------
-- user：用户表（三期注册/登录）
-- ------------------------------------------------------------
DROP TABLE IF EXISTS user;
CREATE TABLE user (
    id              INT UNSIGNED    NOT NULL AUTO_INCREMENT PRIMARY KEY,
    username        VARCHAR(64)     NOT NULL,
    cryptpasswd     VARCHAR(128)    NOT NULL COMMENT 'crypt() 产物，含 salt',
    pwd             INT UNSIGNED    NOT NULL DEFAULT 0 COMMENT '当前虚拟路径的 virtual_file.id，0 表示顶层',
    create_time     DATETIME        NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uk_username (username)
) ENGINE=InnoDB COMMENT='用户表';

-- ------------------------------------------------------------
-- virtual_file：虚拟文件表（PPT 42-43）
-- ------------------------------------------------------------
DROP TABLE IF EXISTS virtual_file;
CREATE TABLE virtual_file (
    id              INT UNSIGNED    NOT NULL AUTO_INCREMENT PRIMARY KEY,
    parent_id       INT UNSIGNED    NOT NULL DEFAULT 0 COMMENT '父目录，0=用户顶层',
    filename        VARCHAR(255)    NOT NULL,
    owner_id        INT UNSIGNED    NOT NULL COMMENT '外键 -> user.id',
    sha256          CHAR(64)        NOT NULL DEFAULT '' COMMENT '对应物理文件的 sha256，目录为空串',
    filesize        BIGINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '字节数，目录为 0',
    type            TINYINT         NOT NULL COMMENT '0=目录, 1=普通文件',
    create_time     DATETIME        NOT NULL DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_parent (parent_id),
    INDEX idx_owner (owner_id),
    KEY  idx_owner_parent (owner_id, parent_id)
) ENGINE=InnoDB COMMENT='虚拟文件表';

-- ------------------------------------------------------------
-- file：物理文件表（秒传 + 引用计数）
-- ------------------------------------------------------------
DROP TABLE IF EXISTS file;
CREATE TABLE file (
    sha256          CHAR(64)        NOT NULL PRIMARY KEY COMMENT '物理文件 sha256，也是磁盘文件名',
    filesize        BIGINT UNSIGNED NOT NULL,
    ref_count       INT UNSIGNED    NOT NULL DEFAULT 1 COMMENT '被多少个 virtual_file 引用',
    create_time     DATETIME        NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB COMMENT='物理文件元信息（秒传/彻底删除依据）';

