def collatz(limit):
    total_steps = 0
    for i in range(1, limit):
        n = i
        steps = 0
        while n > 1:
            if n % 2 == 0:
                n = n // 2
            else:
                n = 3 * n + 1
            steps = steps + 1
        total_steps = total_steps + steps
    return total_steps


def main():
    print("Collatz sequence")
    res = collatz(1000000)
    print(res)


main()
