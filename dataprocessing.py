import matplotlib.pyplot as plt
from matplotlib import colors
from matplotlib.ticker import PercentFormatter
import numpy as np
import sys
import getopt
import seaborn as sns

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
    linputfile = ''
    rinputfile = ''
    oname = 'sample'
    plot_title = 'Histogram Plot'
    x_axis = 'Milliseconds [ms]'
    y_axis = ''

    try:
        opts, args = getopt.getopt(argv,"hl:r:o:",longopts=["help","lifile=","rifile=","oname="])
    except getopt.GetoptError as err:
        print('dataprocessing.py -l <left_inputfile> -r <right_inputfile> -o <outputname>')
        sys.exit(2)

    for opt,arg in opts:
        if opt in ('-h', '--help'):
            print('dataprocessing.py -l <left_inputfile> -r <right_inputfile> -o <outputname>')
            sys.exit()
        elif opt in ('-l', '--lifile'):
            linputfile = arg
        elif opt in ('-r', '--rifile'):
            rinputfile = arg
        elif opt in ('-o', '--oname'):
            oname = arg
    
    # Load the data from the files
    data_file1 = open(linputfile, 'r')
    data_left = data_file1.readlines()
    data_file1.close()
    data_file2 = open(rinputfile, 'r')
    data_right = data_file2.readlines()
    data_file2.close()

    # ------- HISTOGRAM -------
    # Set number of columns
    n_bins = 40

    # Creating a DTU Colormap
    cmap = colors.LinearSegmentedColormap.from_list('DTU_Colors', clrs, n_bins)

    # Save the data in numpy arrays
    dist_left = np.array(data_left,dtype=np.float64)
    dist_right = np.array(data_right, dtype=np.float64)

    fig, (axis_left, axis_right) = plt.subplots(1,2,figsize=(16,6), sharex=True, sharey=True)
    fig.suptitle("Histogram " + oname)
    bins = np.linspace(-50, 150, n_bins)

    # Left histogram
    N_left, bins_left, patches_left = axis_left.hist(dist_left, bins=bins, density=True)
    axis_left.set_title("Error of Benchmark")
    axis_left.set_xlabel("Milliseconds [ms]")
    axis_left.set_ylabel("Density")
    # Right histogram
    N_right, bins_right, patches_right = axis_right.hist(dist_right, bins=bins, density=True)
    axis_right.set_title("Error of Prediction")
    axis_right.set_xlabel("Milliseconds [ms]")
    #axis_right.set_ylabel("Density")

    # Color with regards to density
    fracs_left = N_left/N_left.max()
    norm_left = colors.Normalize(fracs_left.min(), fracs_left.max())
    for thisfrac, thispatch in zip(fracs_left, patches_left):
        color = cmap(norm_left(thisfrac))
        thispatch.set_facecolor(color)
    axis_left.yaxis.set_major_formatter(PercentFormatter(1))
    
    fracs_right = N_right/N_right.max()
    norm_right = colors.Normalize(fracs_right.min(), fracs_right.max())
    for thisfrac, thispatch in zip(fracs_right, patches_right):
        color = cmap(norm_right(thisfrac))
        thispatch.set_facecolor(color)
    axis_right.yaxis.set_major_formatter(PercentFormatter(1))

    plt.tight_layout()
    #plt.show()
    # Save the plot to a pdf
    plt.savefig(oname + "_hist.pdf")
    plt.clf()
    fig = plt.figure(figsize=(8,6))

    # ------ KDE PLOT ------
    plt.title("KDE Plot " + oname)
    sns.kdeplot(dist_left, shade = True, linewidth = 1, clip = (-60, 150),color=clrs[2], label="Error of Benchmark")
    sns.kdeplot(dist_right, shade = True, linewidth = 1, clip = (-60, 150),color=clrs[-3], label="Error of Prediction")
    plt.xticks(np.arange(-60,150,20))
    plt.xlabel("Milliseconds [ms]")
    plt.ylabel("Density")
    plt.legend(loc=2)
    #plt.show()
    plt.savefig(oname + "_KDE.pdf")
    plt.clf()

    # ------ CDF PLOT ------
    plt.title("CDF Plot " + oname)
    sns.kdeplot(dist_left, shade = True, linewidth = 1, clip = (-50, 150),color=clrs[2], cumulative=True, label="Error of Benchmark")
    sns.kdeplot(dist_right, shade = True, linewidth = 1, clip = (-50, 150),color=clrs[-3], cumulative=True, label="Error of Prediction")
    plt.xlabel("Milliseconds [ms]")
    plt.ylabel("Density")
    plt.legend(loc=2)
    #plt.show()
    plt.savefig(oname + "_CDF.pdf")
    plt.clf()

    # ------ BOX PLOT -------
    plt.title("Box Plot " + oname)
    sns.boxplot(data=[dist_left, dist_left],
        palette=[clrs[2], clrs[-3]]
    )

    plt.tick_params(labelbottom=True, labeltop=False, labelleft=True, labelright=True,
                     bottom=True, top=False, left=True, right=True)
    plt.yticks(np.arange(-50,400,25))
    plt.ylabel("Milliseconds [ms]")
    plt.xticks([0,1],["Error of Benchmark", "Error of Prediction"])
    #plt.show()
    plt.savefig(oname + "_box.pdf")
    plt.clf()

    return

if __name__ == "__main__":
	main(sys.argv[1:])