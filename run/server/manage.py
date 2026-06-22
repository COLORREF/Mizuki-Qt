# Django 项目管理入口
# 启动命令: ..\python\python.exe manage.py runserver 0.0.0.0:8000

import os
import sys

# embeddable Python 默认不包含当前目录，需手动添加
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

def main():
    os.environ.setdefault("DJANGO_SETTINGS_MODULE", "blog_backend.settings")
    from django.core.management import execute_from_command_line
    execute_from_command_line(sys.argv)

if __name__ == "__main__":
    main()
