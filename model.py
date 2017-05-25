import argparse
from collections import defaultdict
import gzip
import logging
import sys

from keras import models, layers, regularizers, optimizers
from keras.utils import plot_model
import numpy


def setup():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True,
                        help="Path to the train data")
    parser.add_argument("--output", required=True,
                        help="Path where the model will be saved")
    parser.add_argument("--maxlen", type=int, default=100)
    parser.add_argument("--validation", type=float, default=0)
    parser.add_argument("--neurons", type=int, default=128)
    parser.add_argument("--dense-neurons", type=int, default=0)
    parser.add_argument("--learning-rate", type=float, default=0.001)
    parser.add_argument("--type", default="LSTM")
    parser.add_argument("--dropout", type=float, default=0)
    parser.add_argument("--regularization", type=float, default=0)
    parser.add_argument("--recurrent-dropout", type=float, default=0)
    parser.add_argument("--activation", default="tanh")
    parser.add_argument("--optimizer", default="rmsprop")
    parser.add_argument("--batch-size", type=int, default=128)
    parser.add_argument("--epochs", type=int, default=50)
    parser.add_argument("--visualize")
    args = parser.parse_args()
    logging.basicConfig(level=logging.INFO)
    return args


def main():
    args = setup()
    log = logging.getLogger("main")
    log.info("parsing %s...", args.input)
    threads = defaultdict(list)
    ptrs = {}
    with gzip.open(args.input) as fin:
        for line in fin:
            parts = line[:-1].split(b"\t")
            thread = int(parts[1])
            size = int(parts[2])
            ptr = parts[3]
            if size > -1:
                threads[thread].append(size.bit_length())
                ptrs[ptr] = size
            else:
                size = ptrs.get(ptr, 0)
                if size > 0:
                    del ptrs[ptr]
                threads[thread].append(32 + size.bit_length())
    log.info("generating the training samples...")
    maxlen = args.maxlen
    train_size = sum(max(0, len(v) - maxlen) for v in threads.values())
    log.info("number of samples: %d", train_size)
    try:
        x = numpy.zeros((train_size, maxlen), dtype=numpy.int8)
    except MemoryError as e:
        log.error("failed to allocate %d bytes", train_size * maxlen)
        raise e from None
    y = numpy.zeros((train_size, 64), dtype=numpy.int8)
    offset = 0
    for _, allocs in sorted(threads.items()):
        for i in range(maxlen, len(allocs)):
            x[offset] = allocs[i - maxlen:i]
            y[offset, allocs[i].bit_length()] = 1
            offset += 1
    log.info("training...")
    model = train(x, y, **args.__dict__)
    log.info("writing %s...", args.output)
    model.save(args.output, overwrite=True)


def train(x, y, **kwargs):
    neurons = kwargs.get("neurons", 128)
    dense_neurons = kwargs.get("dense_neurons", 0)
    learning_rate = kwargs.get("learning_rate", 0.001)
    dropout = kwargs.get("dropout", 0)
    recurrent_dropout = kwargs.get("recurrent_dropout", 0)
    optimizer = kwargs.get("optimizer", "rmsprop")
    regularization = kwargs.get("regularization", 0)
    batch_size = kwargs.get("batch_size", 128)
    epochs = kwargs.get("epochs", 50)
    layer_type = kwargs.get("type", "LSTM")
    validation = kwargs.get("validation", 0)
    model = models.Sequential()
    embedding = numpy.zeros((64, 64), dtype=numpy.float32)
    numpy.fill_diagonal(embedding, 1)
    model.add(layers.embeddings.Embedding(
        64, 64, input_length=x[0].shape[-1], weights=[embedding],
        trainable=False))
    model.add(getattr(layers, layer_type)(
        neurons, dropout=dropout, recurrent_dropout=recurrent_dropout,
        kernel_regularizer=regularizers.l2(regularization),
        return_sequences=True))
    model.add(getattr(layers, layer_type)(
        neurons // 2, dropout=dropout, recurrent_dropout=recurrent_dropout,
        kernel_regularizer=regularizers.l2(regularization)))
    if dense_neurons > 0:
        model.add(layers.Dense(dense_neurons))
        model.add(layers.normalization.BatchNormalization())
        model.add(layers.advanced_activations.PReLU())
    model.add(layers.Dense(y[0].shape[-1], activation="softmax"))
    optimizer = getattr(optimizers, optimizer)(lr=learning_rate, clipnorm=1.)
    model.compile(loss="categorical_crossentropy", optimizer=optimizer,
                  metrics=["accuracy", "top_k_categorical_accuracy"])
    if kwargs.get("visualize"):
        plot_model(model, to_file=kwargs["visualize"], show_shapes=True)
    model.fit(x, y, batch_size=batch_size, epochs=epochs,
              validation_split=validation)
    return model


if __name__ == "__main__":
    sys.exit(main())
