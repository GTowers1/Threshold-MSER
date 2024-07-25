import sqlite3
import sys


def insert_data(conn, data):
    cursor = conn.cursor()
    cursor.execute('''
    INSERT INTO Labels (column1, column2, column3)
    VALUES (?, ?, ?)
    ''', data)
    conn.commit()

def read_file_and_store_in_db(filename, db_name):
    conn = sqlite3.connect(db_name)

    with open(filename, 'r') as file:
        for line in file:
            data = line.strip().split(' ')
            insert_data(conn, data)

    conn.close()

if __name__ == '__main__':
    if len(sys.argv) != 5:
        print("Usage: python insertdb.py -d <dbfile> -i <imgfile>")
        sys.exit(1)

    db_name = None
    file_name = None

    for i in range(1, len(sys.argv)):
        if sys.argv[i] == '-d':
            db_name = sys.argv[i+1]
        elif sys.argv[i] == '-i':
            file_name = sys.argv[i+1]

    if not db_name or not file_name:
        print("Usage: python insertdb.py -d <dbfile> -i <imgfile>")
        sys.exit(1)

    read_file_and_store_in_db(file_name, db_name)

