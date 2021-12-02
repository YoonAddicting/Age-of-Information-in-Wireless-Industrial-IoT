import matplotlib.pyplot as plt
from matplotlib import colors
from matplotlib.ticker import PercentFormatter
import numpy as np
import sys
import getopt
import pandas as pd

clrs = [
          (121/255, 35/255, 142/255), # Purple
          (3/255, 15/255, 79/255), # Navy Blue
          (47/255, 62/255, 234/255), # Blue
          (0, 136/255, 53/255), #Green
          (31/255, 208/255, 130/255), # Bright green
          (246/255, 208/255, 77/255), # Yellow
          (252/255, 118/255, 52/255), # Orange
          (232/255, 63/255, 72/255), # Red
          (153/255, 0, 0), #DTU Red
          ]

def main(argv):
    # Handle input arguments
    inputfile = ''
    outputfile = 'test.pdf'
    plot_title = 'Histogram Plot'
    x_axis = ''
    y_axis = ''

    try:
        opts, args = getopt.getopt(argv,"ht:x:y:i:o:",longopts=["help","title=","x-axis=","y-axis=","ifile=","ofile="])
    except getopt.GetoptError as err:
        print('dataprocessing.py -i <inputfile> -o <outputfile>')
        sys.exit(2)

    for opt,arg in opts:
        if opt in ('-h', '--help'):
            print('dataprocessing.py -i <inputfile> -o <outputfile> -t <title> -x <x-axis name> -y <y-axis name>')
            sys.exit()
        elif opt in ('-i', '--ifile'):
            inputfile = arg
        elif opt in ('-o', '--ofile'):
            outputfile = arg
        elif opt in ('-t', '--title'):
            plot_title = arg
        elif opt in ('-x', '--x-axis'):
            x_axis = arg
        elif opt in ('-y', '--y-axis'):
            y_axis = arg
    
    print("input file: {}, output file: {}, title: {}, x-axis: {}, y-axis: {}".format(inputfile,outputfile,plot_title,x_axis,y_axis))

    # Set number of columns
    n_bins = 10

    # Creating a DTU Colormap
    cmap = colors.LinearSegmentedColormap.from_list('DTU_Colors', clrs, n_bins)

    if inputfile == '':
        # Generate random data for sample
        rng = np.random.default_rng(19680801)
        N_points = 100000
        dist = 10* rng.standard_normal(N_points)
    else:
        # Load the data from the file and save in dist
        data_file = open(inputfile, 'r')
        data = data_file.readlines()
        dist = np.array(data,dtype=np.float64)
        data_file.close()

    

    # Do the plot
    N, bins, patches = plt.hist(dist, bins=n_bins, density=True)
    plt.title(plot_title)
    plt.xlabel(x_axis)
    plt.ylabel(y_axis)

    # Normalize and plot with y-axis being % of total
    fracs = N/N.max()
    norm = colors.Normalize(fracs.min(), fracs.max())
    for thisfrac, thispatch in zip(fracs, patches):
        color = cmap(norm(thisfrac))
        thispatch.set_facecolor(color)
    plt.gca().yaxis.set_major_formatter(PercentFormatter(1))
    
    #plt.show()

    # Save the plot to a pdf
    plt.savefig(outputfile)
    return

if __name__ == "__main__":
	main(sys.argv[1:])