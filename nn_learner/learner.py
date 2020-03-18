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

        self.n_layers = 10
        self.relu = Activation('relu')
        self.dense = [Dense(100) for _ in range(self.n_layers)]

        self.dense_middle1 = Dense(1)
        self.dense_middle2 = Dense(1)
        self.dense_final = Dense(1)

        self.normalization = BatchNormalization()

    def call(self, x):
        stops = {self.n_layers // 3 : self.dense_middle1, 2 * self.n_layers // 3: self.dense_middle2}
        outputs = []
        for i in range(self.n_layers):
            x = self.dense[i](x)
            x = self.relu(x)
            if i in stops.keys():
                outputs.append(stops[i](x))
                x = self.normalization(x)
        x = self.dense_final(x)
        outputs.append(x)
        return outputs


@tf.function
def train_step(model, samples, labels):
    with tf.GradientTape() as tape:
        predictions = model(samples)

        loss = 0
        for predictions_i in predictions:
            loss += loss_object(labels, predictions_i)
        loss /= len(predictions)

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
    samples = samples[permutation].astype(np.float32)
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
    train(train_dataset, dataset_test, model, 10)

    results_df = pd.DataFrame({'real': labels_test})
    results_lst = model(samples_test)
    for i, results in enumerate(results_lst):
        title_pred = f'pred{i}'
        title_diff = f'diff{i}'
        results_df[title_pred] = results.numpy()[:, 0]
        results_df[title_diff] = np.abs(results_df['real'] - results_df[title_pred])
        print(f'mu_{i} = {np.mean(results_df[title_diff])}')