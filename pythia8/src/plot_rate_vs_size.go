package main

import (
	"encoding/csv"
	"flag"
	"fmt"
	"image/color"
	"log"
	"math"
	"os"
	"sort"
	"strconv"
	"strings"

	"gonum.org/v1/plot"
	"gonum.org/v1/plot/plotter"
	"gonum.org/v1/plot/vg"
	"gonum.org/v1/plot/vg/draw"
)

func printUsage() {
	fmt.Fprintf(os.Stderr, `Usage: `+os.Args[0]+` [options] <csv-input-files>...
options:
`,
	)
	flag.PrintDefaults()
}

var (
	output    = flag.String("output", "out.png", "output file")
	logYScale = flag.Bool("logy", false, "logarithmic Y scale")
)

type XY struct{ X, Y float64 }

func main() {
	flag.Usage = printUsage
	flag.Parse()
	if flag.NArg() < 1 {
		printUsage()
		log.Fatal("Invalid arguments")
	}

	axisFont, _ := vg.MakeFont("Times-Roman", 10)
	tickFont, _ := vg.MakeFont("Times-Roman", 8)
	legendFont, _ := vg.MakeFont("Times-Roman", 8)

	p, _ := plot.New()
	p.X.Label.Text = "File Size (MB)"
	p.X.Label.Font = axisFont
	p.Y.Label.Text = "Event Rate (Hz)"
	p.Y.Label.Font = axisFont
	p.X.Min = 0
	p.Y.Min = 0
	p.X.Tick.Marker = PreciseTicks{NSuggestedTicks: 4}
	p.Y.Tick.Marker = PreciseTicks{NSuggestedTicks: 4}
	p.X.Tick.Label.Font = tickFont
	p.Y.Tick.Label.Font = tickFont
	p.Legend.Font = legendFont

	if *logYScale {
		p.Y.Scale = LogScale{}
		p.Y.Tick.Marker = LogTicks{}
		p.Y.Min = math.Inf(1)
	}

	for i := 0; i < flag.NArg(); i++ {
		csvPath := flag.Arg(i)
		csvFile, _ := os.Open(csvPath)
		csvReader := csv.NewReader(csvFile)
		records, _ := csvReader.ReadAll()

		var pts plotter.XYs
		var intPts plotter.XYs
		labelSuffix := "proio"
		if strings.HasSuffix(csvPath, "_root.csv") {
			labelSuffix = "root"
		}
		for _, record := range records {
			var pt XY
			pt.X, _ = strconv.ParseFloat(strings.Replace(record[1], " ", "", -1), 64)
			pt.X /= (1 << 20)
			pt.Y, _ = strconv.ParseFloat(strings.Replace(record[2], " ", "", -1), 64)
			if strings.HasPrefix(record[0], "packed_") {
				pts = append(pts, pt)
			} else {
				intPts = append(intPts, pt)
			}
		}
		sort.Slice(pts, func(i, j int) bool { return pts[i].X < pts[j].X })
		sort.Slice(intPts, func(i, j int) bool { return intPts[i].X < intPts[j].X })

		l, s, _ := plotter.NewLinePoints(pts)
		l.LineStyle.Width = 0.5
		intL, intS, _ := plotter.NewLinePoints(intPts)
		intL.LineStyle.Width = 0.5
		if labelSuffix == "proio" {
			pointColor := color.RGBA{B: 255, A: 255}
			s.GlyphStyle.Shape = draw.CircleGlyph{}
			s.GlyphStyle.Color = pointColor
			intS.GlyphStyle.Shape = draw.RingGlyph{}
			intS.GlyphStyle.Color = pointColor
			lineColor := color.RGBA{B: 255, A: 50}
			l.LineStyle.Color = lineColor
			intL.LineStyle.Color = lineColor
		} else {
			pointColor := color.RGBA{A: 255}
			s.GlyphStyle.Shape = draw.PlusGlyph{}
			s.GlyphStyle.Color = pointColor
			intS.GlyphStyle.Shape = draw.CrossGlyph{}
			intS.GlyphStyle.Color = pointColor
			lineColor := color.RGBA{A: 50}
			l.LineStyle.Color = lineColor
			intL.LineStyle.Color = lineColor
		}

		p.Add(l, s, intL, intS)
		p.Legend.Add(labelSuffix, l, s)
		p.Legend.Add("integer "+labelSuffix, intL, intS)
	}

	p.Save(3*vg.Inch, 3*vg.Inch, *output)
}

type PreciseTicks struct {
	NSuggestedTicks int
}

func (t PreciseTicks) Ticks(min, max float64) []plot.Tick {
	if t.NSuggestedTicks == 0 {
		t.NSuggestedTicks = 4
	}

	if max <= min {
		panic("illegal range")
	}

	tens := math.Pow10(int(math.Floor(math.Log10(max - min))))
	n := (max - min) / tens
	for n < float64(t.NSuggestedTicks)-1 {
		tens /= 10
		n = (max - min) / tens
	}

	majorMult := int(n / float64(t.NSuggestedTicks-1))
	switch majorMult {
	case 7:
		majorMult = 6
	case 9:
		majorMult = 8
	}
	majorDelta := float64(majorMult) * tens
	val := math.Floor(min/majorDelta) * majorDelta
	// Makes a list of non-truncated y-values.
	var labels []float64
	for val <= max {
		if val >= min {
			labels = append(labels, val)
		}
		val += majorDelta
	}
	prec := int(math.Ceil(math.Log10(val)) - math.Floor(math.Log10(majorDelta)))
	// Makes a list of big ticks.
	var ticks []plot.Tick
	for _, v := range labels {
		vRounded := round(v, prec)
		ticks = append(ticks, plot.Tick{Value: vRounded, Label: strconv.FormatFloat(vRounded, 'g', -1, 64)})
	}
	minorDelta := majorDelta / 2
	switch majorMult {
	case 3, 6:
		minorDelta = majorDelta / 3
	case 5:
		minorDelta = majorDelta / 5
	}

	val = math.Floor(min/minorDelta) * minorDelta
	for val <= max {
		found := false
		for _, t := range ticks {
			if t.Value == val {
				found = true
			}
		}
		if val >= min && val <= max && !found {
			ticks = append(ticks, plot.Tick{Value: val})
		}
		val += minorDelta
	}
	return ticks
}

type LogTicks struct{}

func (LogTicks) Ticks(min, max float64) []plot.Tick {
	val := math.Pow10(int(log10(min)))
	max = math.Pow10(int(math.Ceil(log10(max))))
	var ticks []plot.Tick
	for val < max {
		for i := 1; i < 10; i++ {
			if i == 1 {
				ticks = append(ticks, plot.Tick{Value: val, Label: strconv.FormatFloat(val, 'e', -1, 64)})
			}
			ticks = append(ticks, plot.Tick{Value: val * float64(i)})
		}
		val *= 10
	}
	ticks = append(ticks, plot.Tick{Value: val, Label: strconv.FormatFloat(val, 'e', -1, 64)})

	return ticks
}

func round(x float64, prec int) float64 {
	if x == 0 {
		// Make sure zero is returned
		// without the negative bit set.
		return 0
	}
	// Fast path for positive precision on integers.
	if prec >= 0 && x == math.Trunc(x) {
		return x
	}
	pow := math.Pow10(prec)
	intermed := x * pow
	if math.IsInf(intermed, 0) {
		return x
	}
	if x < 0 {
		x = math.Ceil(intermed - 0.5)
	} else {
		x = math.Floor(intermed + 0.5)
	}

	if x == 0 {
		return 0
	}

	return x / pow
}

type LogScale struct{}

func (LogScale) Normalize(min, max, x float64) float64 {
	logMin := log10(min)
	return (log10(x) - logMin) / (log10(max) - logMin)
}

func log10(x float64) float64 {
	if x <= 0 {
		return -1
	}
	return math.Log10(x)
}
