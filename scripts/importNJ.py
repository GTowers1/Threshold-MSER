import sqlite3
import sys
import os


#problem with the colored img not being in the file unless you run segmentation tool with the colored img

def insert_data(conn, data):
    cursor = conn.cursor()
    cursor.execute('''
        INSERT INTO Labels (CName, X, Y, W, H, IName)
        VALUES (?, ?, ?, ?, ?, ?)
        ''', data)
    conn.commit()

def read_file_and_store_in_db(project_path, db_name, txt_file):
    conn = sqlite3.connect(os.path.join(project_path, f'{db_name}.db'))

    with open(txt_file, 'r') as file:
        for line in file:
            data = line.strip().split(' ')
            #add in the cpp file It is not like this on the main version(hpc)
            #data.append(img_name)
            insert_data(conn, data)

    conn.close()

def help_msg():

    print("\n\tInsert to NJ, The structure for this python call is insertdb.py then you can use the flags below\n")
    print("\tNjobvu Decision, -n , Example calls -n new or -n existing \n")
    print("\tBounding Box text file, -t , Example -t <bbox.txt>, should find in {segmentation output}/measurements/{imgname}_bbox.txt\n")
    print("\tColored Image, -i , Example -i <coloredimg.png> \n")
    print("\tPath to Njobvu Install, -p , Example -p <../Njobvu-AI>, where you cloned Njobvu onto your machine \n")
    print("\tExample call: python3 -n <new or existing> -i <img.png> -t <bbox.txt> -p <../Njobvu-AI>\n")
    print("\n")

def findNjProject(nj_project, nj_path):
    #insert based on an input for nj path
    #this might not work
    base_path = nj_path

    project_path = os.path.join(base_path,"public/projects" ,nj_project)

    if os.path.exists(project_path) and os.path.exists(os.path.join(project_path, f'{nj_project}.db')):
        print(f"Project {nj_project} found at {project_path}")
        return project_path + ".db"
    else:
        print(f"Project {nj_project} not found at {project_path}")
        return ""


def create_project(db_name, txt_file, nj_path):

    # Creates the project dir
    # use cmd to cp og img from input path to the images dir
    #update this to put it all under a public/project tab in njobvu using the nj_path var TANISH (DONE)

    # Points to path where project needs to be placed

    # Inserts the project dir
    project_path = os.path.join(nj_path, db_name)

    if not os.path.exists(project_path):
        os.makedirs(project_path)

    os.makedirs(os.path.join(project_path, 'bootstrap'))
    os.makedirs(os.path.join(project_path, 'images'))
    os.makedirs(os.path.join(project_path, 'Training'))

    conn = sqlite3.connect(os.path.join(project_path, f'{db_name}.db'))
    conn.close()
    print(f"New project created with directory and database: {project_path}")

    # populating the database

    conn = sqlite3.connect(os.path.join(project_path, f'{db_name}.db'))
    cursor = conn.cursor()

    cursor.execute('''
    CREATE TABLE Classes (CName VARCHAR NOT NULL PRIMARY KEY)
    ''')

    cursor.execute('''
    CREATE TABLE Images (IName VARCHAR NOT NULL PRIMARY KEY, reviewImage INTEGER NOT NULL DEFAULT 0, validateImage INTEGER NOT NULL DEFAULT 0)
    ''')

    cursor.execute('''
    CREATE TABLE Labels (LID INTEGER PRIMARY KEY, CName VARCHAR NOT NULL, X INTEGER NOT NULL, Y INTEGER NOT NULL, W INTEGER NOT NULL, H INTEGER NOT NULL, IName VARCHAR NOT NULL, FOREIGN KEY(CName) REFERENCES Classes(CName), FOREIGN KEY(IName) REFERENCES Images(IName))
    ''')

    cursor.execute('''
    CREATE TABLE Validation (Confidence INTEGER NOT NULL, LID INTEGER NOT NULL PRIMARY KEY, CName VARCHAR NOT NULL, IName VARCHAR NOT NULL, FOREIGN KEY(LID) REFERENCES Labels(LID), FOREIGN KEY(IName) REFERENCES Images(IName), FOREIGN KEY(CName) REFERENCES Classes(CName))
    ''')

    read_file_and_store_in_db(project_path, db_name, txt_file)

if __name__ == '__main__':

    db_name = None
    img_name = None
    txt_file = None
    nj_decision = None
    nj_path = None


    for i in range(1, len(sys.argv)):
        if sys.argv[i] == '-n':
            if sys.argv[i + 1] != ("new" or "existing"):
                print("error in -n flag input: required input = 'new' or 'existing'\n")
            else:
                nj_decision = sys.argv[i+1]
        elif sys.argv[i] == '-i':
            img_name = sys.argv[i+1]
        elif sys.argv[i] == '-t':
            txt_file = sys.argv[i+1]
        elif sys.argv[i] == '-p':
            nj_path = sys.argv[i+1]
        elif sys.argv[i] == '-h':
            help_msg()
        else:
            print("that was not one of the options these are all the options\n\n")
            help_msg()

    if nj_decision == "new":
        db_name = input("Enter the name of the project: ")
        create_project(db_name, txt_file, nj_path)


    elif nj_decision == "existing":
        nj_project = input("Enter the name of your existing Njobvu project: ")
        result = findNjProject(nj_project,nj_path)
        if result != "":
            read_file_and_store_in_db(result, txt_file)
