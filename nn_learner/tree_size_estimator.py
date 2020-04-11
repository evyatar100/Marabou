from __future__ import absolute_import, division, print_function, unicode_literals

import os
import sys

import numpy as np
import pandas as pd
import tensorflow as tf
from tensorflow.keras import Model
from tensorflow.keras.layers import Dense, Activation, BatchNormalization

N_CONSTRAINS = 224


class MyModel(Model):

    def __init__(self, n_layers, layer_size, activation):
        super(MyModel, self).__init__()

        self.n_layers = n_layers
        self.relu = Activation(activation)
        self.dense = [Dense(layer_size) for _ in range(self.n_layers)]

        self.dense_middle1 = Dense(1)
        self.dense_middle2 = Dense(1)
        self.dense_final = Dense(1)

        self.normalization = BatchNormalization()

    def call(self, x):
        stops = {self.n_layers // 3: self.dense_middle1, 2 * self.n_layers // 3: self.dense_middle2}
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


class TreeSizeEstimator:

    def __init__(self):
        self.optimizer = tf.keras.optimizers.Adam()
        self.loss_object = tf.keras.losses.MeanSquaredError()
        self.train_loss = tf.keras.metrics.Mean(name='train_loss')
        self.test_loss = tf.keras.metrics.Mean(name='test_loss')

        self.n_layers = 10
        self.layel_size = 100
        self.activation = 'relu'

        self.model = MyModel(self.n_layers, self.layel_size, self.activation)

        self.checkpoint_dir = './training_checkpoints'
        self.checkpoint_prefix = os.path.join(self.checkpoint_dir, "ckpt")
        self.checkpoint = tf.train.Checkpoint(optimizer=self.optimizer, model=self.model)

    # def create_model(self):
    #     model = tf.keras.Sequential()
    #     for i in range(self.n_layers):
    #         model.add(Dense(self.layel_size))
    #         model.add(Activation(self.activation))
    #     model.add(Dense(1))
    #
    #     return model

    @tf.function
    def train_step(self, samples, labels):
        with tf.GradientTape() as tape:
            predictions = self.model(samples)

            loss = 0
            for predictions_i in predictions:
                loss += self.loss_object(labels, predictions_i)
            loss /= len(predictions)

            gradients = tape.gradient(loss, self.model.trainable_variables)
            self.optimizer.apply_gradients(zip(gradients, self.model.trainable_variables))
        self.train_loss(loss)

    @tf.function
    def test_step(self, samples, labels):
        predictions = self.model(samples)
        t_loss = self.loss_object(labels, predictions)
        self.test_loss(t_loss)

    def train(self, train_ds, test_ds, epochs):
        iterations_c = 0
        for epoch in range(epochs):
            for test_images, test_labels in test_ds:
                self.test_step(test_images, test_labels)

            for samples, labels in train_ds:
                iterations_c += 1
                self.train_step(samples, labels)

            print(
                f'Epoch {epoch + 1}, iteration {iterations_c}, Train Loss: {self.train_loss.result()}, '
                f'Test Loss: {self.test_loss.result()}')

            # Reset the metrics for the next epoch
            self.train_loss.reset_states()
            self.test_loss.reset_states()

        self.checkpoint.save(file_prefix=self.checkpoint_prefix)

    @staticmethod
    def get_data_formated(csv_path):
        data = pd.read_csv(csv_path)

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
        dataset_train_one = tf.data.Dataset.from_tensor_slices((samples_train, labels_train_log))
        dataset_test_one = tf.data.Dataset.from_tensor_slices((samples_test, labels_test_log))
        train_dataset = dataset_train_one.shuffle(len(samples[n_test:])).batch(500)
        train_dataset_test = dataset_test_one.shuffle(n_test).batch(n_test)

        return train_dataset, train_dataset_test, samples_test, labels_test

    def train_model(self, csv_path):
        train_dataset, train_dataset_test, samples_test, labels_test = TreeSizeEstimator.get_data_formated(csv_path)

        self.checkpoint.restore(tf.train.latest_checkpoint(self.checkpoint_dir))

        self.train(train_dataset, train_dataset_test, epochs=10)
        #
        # results_df = pd.DataFrame({'real': labels_test})
        # results_lst = self.model(samples_test)
        # for i, results in enumerate(results_lst):
        #     title_pred = f'pred{i}'
        #     title_diff = f'diff{i}'
        #     results_df[title_pred] = results.numpy()[:, 0]
        #     results_df[title_diff] = np.abs(results_df['real'] - results_df[title_pred])
        #     print(f'mu_{i} = {np.mean(results_df[title_diff])}')

    def restore_model(self):
        self.checkpoint.restore(tf.train.latest_checkpoint(self.checkpoint_dir))

    def check_input(self, network_state):
        return network_state.shape == (1, N_CONSTRAINS * 2)

    def get_best_constraint(self, network_state):
        assert network_state.shape == (1, N_CONSTRAINS * 2)

        input_tensor = np.zeros((N_CONSTRAINS, N_CONSTRAINS * 3), dtype=np.float32)
        input_tensor[:, :N_CONSTRAINS * 2] = network_state
        input_tensor[:, N_CONSTRAINS * 2:] = np.eye(N_CONSTRAINS)

        output = np.array(self.model(input_tensor))[-1, :, 0]
        argsort = np.argsort(output)
        return argsort

    input_example = np.array([[0., 0., 1., 1., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1., 0.,
                               1., 1., 1., 1., 0., 0., 0., 0., 0., 0., 1., 1., 0., 0., 0., 0.,
                               0., 0., 0., 0., 0., 0., 0., 0., 1., 1., 0., 0., 1., 1., 0., 0.,
                               0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
                               0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
                               0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
                               0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
                               0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
                               0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1., 1.,
                               0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
                               1., 0., 1., 1., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
                               1., 1., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
                               0., 0., 0., 0., 1., 1., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
                               0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
                               0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
                               1., 0., 1., 0., 1., 0., 0., 0., 1., 0., 0., 0., 0., 0., 0., 0.,
                               0., 0., 1., 1., 1., 1., 0., 0., 1., 0., 1., 1., 0., 0., 0., 0.,
                               1., 0., 0., 0., 0., 0., 1., 1., 0., 0., 0., 0., 0., 0., 1., 0.,
                               0., 0., 1., 0., 1., 1., 0., 0., 1., 0., 0., 0., 0., 0., 0., 0.,
                               0., 0., 0., 0., 0., 0., 0., 0., 1., 1., 0., 0., 0., 0., 1., 0.,
                               0., 0., 0., 0., 0., 0., 1., 0., 1., 0., 1., 0., 0., 0., 0., 0.,
                               0., 0., 0., 0., 1., 0., 0., 0., 0., 0., 1., 0., 1., 0., 1., 1.,
                               1., 0., 1., 0., 1., 0., 1., 0., 1., 0., 1., 0., 1., 0., 1., 0.,
                               1., 0., 1., 0., 1., 0., 1., 0., 1., 0., 1., 0., 1., 0., 1., 0.,
                               1., 0., 1., 0., 1., 0., 1., 0., 1., 0., 1., 0., 1., 0., 1., 0.,
                               1., 0., 1., 0., 1., 0., 0., 0., 1., 0., 1., 0., 1., 0., 1., 0.,
                               1., 0., 1., 0., 1., 0., 1., 0., 1., 0., 1., 0., 1., 0., 1., 0.,
                               1., 0., 1., 0., 1., 0., 1., 0., 1., 0., 1., 0., 1., 0., 1., 0.]])
