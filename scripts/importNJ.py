import sqlite3
import sys


def insert_data(conn, data):
    cursor = conn.cursor()
    cursor.execute('''
    INSERT INTO Labels (CName, X, Y, W, H, IName)
    VALUES (?, ?, ?, ?, ?, ?)
    ''', data)
    conn.commit()

def read_file_and_store_in_db(img_name, db_name, txt_file):
    conn = sqlite3.connect(db_name)

    with open(txt_file, 'r') as file:
        for line in file:
            data = line.strip().split(' ')
            data.append(img_name)
            insert_data(conn, data)

    conn.close()

if __name__ == '__main__':
    if len(sys.argv) != 5:
        print("Usage: python insertdb.py -d <dbfile> -i <imgfile>")
        sys.exit(1)

    db_name = None
    img_name = None
    txt_file = None

    for i in range(1, len(sys.argv)):
        if sys.argv[i] == '-d':
            db_name = sys.argv[i+1]
        elif sys.argv[i] == '-i':
            img_name = sys.argv[i+1]
        elif sys.argv[i] == '-t':
            txt_file = sys.argv[i+1]

    if not db_name or not file_name or not txt_file:
        print("Usage: python insertdb.py -d <dbfile> -i <imgfile> -t <txtfile>")
        sys.exit(1)

    read_file_and_store_in_db(img_name, db_name, txt_file)

