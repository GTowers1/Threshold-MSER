import cv2
import sys
import os.path
import csv
import random
#try to maybe get the bounding box information back to a float if possible, I think the more accurate the better for yolo


def plot_one_box(x, image, line_thickness=None):
    # Plots one bounding box on image img
    tl = line_thickness or round(
        0.002 * (image.shape[0] + image.shape[1]) / 2) + 1  # line/font thickness
    color = (255,255,255)
    c1, c2 = (int(x[0]), int(x[1])), (int(x[2]), int(x[3]))
    cv2.rectangle(image, c1, c2, color, thickness=tl, lineType=cv2.LINE_AA)

    print("Plotting box: c1 =", c1, "c2 =", c2, "color =", color, "line_thickness =", tl)


def draw_bounding_boxes():
    i = 1
    inputPath = ""
    outputPath = ""
    dataPath = ""
    class_idx = 0

    while i < len(sys.argv):
        if sys.argv[i] == "-i":
            if os.path.isfile(sys.argv[i + 1]):
                inputPath = sys.argv[i + 1]
                i += 1
            else:
                print("Input file not found:", sys.argv[i + 1])
                return 1
        elif sys.argv[i] == "-o":
            if os.path.isdir(sys.argv[i + 1]):
                outputPath = sys.argv[i + 1]
                i += 1
            else:
                print("Output directory not found:", sys.argv[i + 1])
                return 1
        elif sys.argv[i] == "-d":
            if os.path.isfile(sys.argv[i + 1]):
                dataPath = sys.argv[i + 1]
                i += 1
            else:
                print("Data file not found:",sys.argv[i + 1])
                return 1
        #add an option to change class label so that it doesn't just work for pteropods

        else:
            print("Help Msg:------------------- \n use -i [input og img file] -o [output directory] -d [data file]\n")


        i += 1

    # get values for each line in the data
    #rows is an array that stores a list of bboxes [x, y, width, height
    rows = []

    file_name = os.path.basename(inputPath)
    save_file_path = os.path.join(outputPath, file_name)
    print(save_file_path)

    image = cv2.imread(inputPath)
    if image is None:
        print("Failed to read image from path:", inputPath)
        return 1


    try:
        height, width, channels = image.shape
    except:
        print('no shape info')
        return 0



    with open(dataPath, 'r') as file:
        csvreader = csv.reader(file)
        header = next(csvreader)
        for row in csvreader:
            rows.append(row)

    for row in rows:
        

        x_center, y_center, w, h = float(row[0])*width, float(row[1])*height, float(row[2])*width, float(row[3])*height

        x1 = round(x_center-w/2)
        y1 = round(y_center-h/2)
        x2 = round(x_center+w/2)
        y2 = round(y_center+h/2)

        print("Bounding box coordinates:", x1, y1, x2, y2,"\n")

        plot_one_box([x1,y1,x2,y2], image, line_thickness=None)

         

    cv2.imwrite(save_file_path, image)
    print("Image with bounding boxes saved to:", save_file_path)
    #take values and calculate a box to annotate

    #use annotation tool to add box to og img

    #



if __name__ == '__main__':

    draw_bounding_boxes()

