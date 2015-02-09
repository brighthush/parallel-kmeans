#coding=utf8
import numpy as np
import matplotlib.pyplot as plt

class ProcData:
    def __init__(self, data_path):
        self.data_path = data_path

    def read_data(self):
        f = open(self.data_path, 'r')
        f.readline()
        line = f.readline()
        line = line.strip()
        items = line.split(' ')
        self.num_clusters = int(items[0])
        self.num_rows = int(items[1])
        self.num_cols = int(items[2])
        f.readline()
        self.cens = []
        for i in range(self.num_clusters):
            line = f.readline()
            line = line.strip()
            items = line.split(' ')
            self.cens.append([float(item) for item in items])
        f.readline()
        self.data = []
        self.clusters = []
        for i in range(self.num_rows):
            line = f.readline()
            items = line.strip().split(' ')
            self.clusters.append(int(items[0]))
            self.data.append([float(item) for item in items[1:]])
        f.close()

    def plot_data(self):
        if self.num_cols != 2:
            print 'The data are not in 2 dimensions.'
            exit()
        x = np.asarray([row[0] for row in self.cens])
        y = np.asarray([row[1] for row in self.cens])
        plt.figure()
        plt.scatter(x, y, c='yellow', marker='*')
        dx = np.asarray([row[0] for row in self.data])
        dy = np.asarray([row[1] for row in self.data])
        for i in range(dx.shape[0]):
            if self.clusters[i] == 0:
                plt.scatter(dx[i:i+1], dy[i:i+1], c='r', marker='o')
            elif self.clusters[i] == 1:
                plt.scatter(dx[i:i+1], dy[i:i+1], c='g', marker='o')
            else:
                plt.scatter(dx[i:i+1], dy[i:i+1], c='b', marker='o')
        fig_name = self.data_path.split('/')[-1]
        fig_name += '.png'
        plt.savefig('./' + fig_name)
        plt.show()

def main():
    iteration = ProcData('../visual/iteration-2')
    iteration.read_data()
    print iteration.num_rows, iteration.num_cols, iteration.num_clusters
    iteration.plot_data()

if __name__ == '__main__':
    main()
