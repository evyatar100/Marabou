from __future__ import absolute_import, division, print_function, unicode_literals
import sys

import numpy as np
import pandas as pd
import tensorflow as tf
from tensorflow.keras import Model
from tensorflow.keras.layers import Dense, Activation, BatchNormalization

optimizer = tf.keras.optimizers.Adam()
loss_object = tf.keras.losses.MeanSquaredError()
train_loss = tf.keras.metrics.Mean(name='train_loss')
test_loss = tf.keras.metrics.Mean(name='test_loss')


class MyModel(Model):
    def __init__(self):
        super(MyModel, self).__init__()

        self.n_layers = 20
        self.relu = Activation('relu')
        self.dense = [Dense(500) for _ in range(self.n_layers)]

        self.dense_final = Dense(1)

    def call(self, x):
        for i in range(self.n_layers):
            x = self.dense[i](x)
            x = self.relu(x)
        x = self.dense_final(x)
        return x


@tf.function
def train_step(model, samples, labels):
    with tf.GradientTape() as tape:
        predictions = model(samples)

        loss = loss_object(labels, predictions)
        gradients = tape.gradient(loss, model.trainable_variables)
        optimizer.apply_gradients(zip(gradients, model.trainable_variables))
    train_loss(loss)


@tf.function
def test_step(model, samples, labels):
    predictions = model(samples)
    t_loss = loss_object(labels, predictions)
    test_loss(t_loss)


def train(train_ds, test_ds, model, epochs):
    iterations_c = 0
    for epoch in range(epochs):
        for test_images, test_labels in test_ds:
            test_step(model, test_images, test_labels)

        for samples, labels in train_ds:
            iterations_c += 1
            train_step(model, samples, labels)

        template = 'Epoch {}, iteration {}, Train Loss: {}, Test Loss: {}'
        print(template.format(epoch + 1,
                              iterations_c,
                              train_loss.result(),
                              test_loss.result()
                              ))
        # Reset the metrics for the next epoch
        train_loss.reset_states()
        test_loss.reset_states()


if __name__ == '__main__':
    data_file_src = sys.argv[1]
    # train_file_path = tf.keras.utils.get_file("train.csv", data_file_src)
    data = pd.read_csv(data_file_src)
    samples = data.drop(columns=['Unnamed: 0', ' sub-tree size']).values
    labels = data[' sub-tree size'].values

    permutation = np.random.permutation(samples.shape[0])
    samples = samples[permutation]
    labels = labels[permutation]

    n_test = int(samples.shape[0] * 0.2)
    samples_train = samples[n_test:]
    labels_train = labels[n_test:]

    samples_test = samples[:n_test]
    labels_test = labels[:n_test]

    labels_train_log = np.log2(labels_train)
    labels_test_log = np.log2(labels_test)

    dataset_train = tf.data.Dataset.from_tensor_slices((samples_train, labels_train_log))
    dataset_test = tf.data.Dataset.from_tensor_slices((samples_test, labels_test_log))

    train_dataset = dataset_train.shuffle(len(samples[n_test:])).batch(500)
    dataset_test = dataset_test.shuffle(n_test).batch(n_test)

    model = MyModel()
    train(train_dataset, dataset_test, model, 30)

    results = 2 ** model(samples_test)
    results_df = pd.DataFrame({'real': labels_test, 'pred': results.numpy()[:, 0]})
    results_df['diff'] = np.abs(results_df['real'] - results_df['pred'])
    mu = np.mean(results_df['diff'])
    sig = np.var(results_df['diff'])

    print(mu)
