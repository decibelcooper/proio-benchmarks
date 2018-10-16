package main

import (
	"encoding/csv"
	"math"
	"os"
	"strconv"
	"strings"

	"gonum.org/v1/plot"
	"gonum.org/v1/plot/plotter"
	"gonum.org/v1/plot/vg"
	"gonum.org/v1/plot/vg/draw"
)

type XY struct{ X, Y float64 }

func main() {
	axisFont, _ := vg.MakeFont("Times-Roman", 10)
	tickFont, _ := vg.MakeFont("Times-Roman", 8)

	p, _ := plot.New()
	p.X.Label.Text = "File Size (Bytes)"
	p.X.Label.Font = axisFont
	p.Y.Label.Text = "Event Rate (Hz)"
	p.Y.Label.Font = axisFont
	p.X.Min = 0
	p.Y.Min = 0
	p.X.Tick.Marker = PreciseTicks{}
	p.Y.Tick.Marker = PreciseTicks{}
	p.X.Tick.Label.Font = tickFont
	p.Y.Tick.Label.Font = tickFont

	for i := 1; i < len(os.Args); i++ {
		csvPath := os.Args[i]
		csvFile, _ := os.Open(csvPath)
		csvReader := csv.NewReader(csvFile)
		records, _ := csvReader.ReadAll()

		var pts plotter.XYs
		var intPts plotter.XYs
		for _, record := range records {
			var pt XY
			pt.X, _ = strconv.ParseFloat(strings.Replace(record[1], " ", "", -1), 64)
			pt.Y, _ = strconv.ParseFloat(strings.Replace(record[2], " ", "", -1), 64)
			if strings.HasPrefix(record[0], "packed_") {
				pts = append(pts, pt)
			} else {
				intPts = append(intPts, pt)
			}
		}

		s, _ := plotter.NewScatter(pts)
		intS, _ := plotter.NewScatter(intPts)
		switch i {
		case 1:
			s.GlyphStyle.Shape = draw.CircleGlyph{}
			intS.GlyphStyle.Shape = draw.RingGlyph{}
		case 2:
			s.GlyphStyle.Shape = draw.PlusGlyph{}
			intS.GlyphStyle.Shape = draw.CrossGlyph{}
		}

		p.Add(s)
		p.Add(intS)
	}

	p.Save(3*vg.Inch, 3*vg.Inch, "out.pdf")
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
		ticks = append(ticks, plot.Tick{Value: vRounded, Label: formatFloatTick(vRounded, -1)})
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

func formatFloatTick(v float64, prec int) string {
	return strconv.FormatFloat(v, 'g', prec, 64)
}
