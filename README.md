
Wire Boost.Polygon Voronoi builder + produce raw cells (polygons).
Render raw polygons in HTML (quick visual feedback).
Add Lloyd relaxation (1–3 iterations) to improve cell shapes.
Implement a simple elevation assigner (distance-based island mask + random mountain seeds).
Visualize elevation coloring (we already made elevation-based coloring for tiles; reuse it but for cells).
Add rivers (flow accumulation).
Add moisture and biome rules.
Export JSON.



## Selectively Importing Files from Another GitHub Repo (Sparse Checkout)

This project uses Git sparse checkout to pull only a few specific source files from the [FantasyMapGenerator](https://github.com/ncanderson/FantasyMapGenerator) repository.

Sparse checkout allows you to clone a repository but include only the files or folders you actually need.
This is ideal for pulling in small utilities (like `dcel.h` or `voronoi.cpp`) without bringing the entire repo.

---

### Step-by-Step Instructions

1. Create the external directory

```
mkdir -p external
cd external
```

2. Clone the repository without checking out files

```
git clone --no-checkout https://github.com/ncanderson/FantasyMapGenerator.git
cd FantasyMapGenerator
```

This sets up the repository structure, but no files are actually downloaded yet.

3. Enable sparse checkout in non-cone mode

```
git sparse-checkout init --no-cone
```

Cone mode only supports directory-level filtering.
Non-cone mode allows you to specify individual files, which is what we want.

4. Specify which files to pull

```
git sparse-checkout set \
  /src/utils/dcel.h \
  /src/utils/dcel.cpp \
  /src/voronoi/voronoi.h \
  /src/voronoi/voronoi.cpp
```

You can add more files later with:

```
git sparse-checkout add /src/utils/whatever.cpp
```

To see the current list:

```
git sparse-checkout list
```

5. Checkout the branch and fetch those files

```
git checkout main
```

At this point, only the selected files and their containing directories will appear in your working tree.

6. (Optional) Update the files later

```
git pull origin main
```

7. Remove or reset sparse checkout

To go back to a full checkout:

```
git sparse-checkout disable
```

To remove the local copy entirely:

```
cd ..
rm -rf FantasyMapGenerator
```
